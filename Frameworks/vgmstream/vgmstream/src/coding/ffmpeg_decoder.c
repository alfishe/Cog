#include "../vgmstream.h"

#ifdef VGM_USE_FFMPEG

static void convert_audio(sample *outbuf, const uint8_t *inbuf, int sampleCount, int bitsPerSample, int floatingPoint) {
    int s;
    switch (bitsPerSample) {
        case 8:
        {
            for (s = 0; s < sampleCount; ++s) {
                *outbuf++ = ((int)(*(inbuf++))-0x80) << 8;
            }
        }
            break;
        case 16:
        {
            int16_t *s16 = (int16_t *)inbuf;
            for (s = 0; s < sampleCount; ++s) {
                *outbuf++ = *(s16++);
            }
        }
            break;
        case 32:
        {
            if (!floatingPoint) {
                int32_t *s32 = (int32_t *)inbuf;
                for (s = 0; s < sampleCount; ++s) {
                    *outbuf++ = (*(s32++)) >> 16;
                }
            }
            else {
                float *s32 = (float *)inbuf;
                for (s = 0; s < sampleCount; ++s) {
                    float sample = *s32++;
                    int s16 = (int)(sample * 32768.0f);
                    if ((unsigned)(s16 + 0x8000) & 0xFFFF0000) {
                        s16 = (s16 >> 31) ^ 0x7FFF;
                    }
                    *outbuf++ = s16;
                }
            }
        }
            break;
        case 64:
        {
            if (floatingPoint) {
                double *s64 = (double *)inbuf;
                for (s = 0; s < sampleCount; ++s) {
                    double sample = *s64++;
                    int s16 = (int)(sample * 32768.0f);
                    if ((unsigned)(s16 + 0x8000) & 0xFFFF0000) {
                        s16 = (s16 >> 31) ^ 0x7FFF;
                    }
                    *outbuf++ = s16;
                }
            }
        }
            break;
    }
}

void decode_ffmpeg(VGMSTREAM *vgmstream,
        sample * outbuf, int32_t samples_to_do, int channels) {
    ffmpeg_codec_data *data = (ffmpeg_codec_data *) vgmstream->codec_data;
    
    int frameSize;
    int dataSize;
    
    int bytesToRead;
    int bytesRead;
    
    int errcode;
    
    uint8_t *targetBuf;
    
    AVFormatContext *formatCtx;
    AVCodecContext *codecCtx;
    AVPacket *lastReadPacket;
    AVFrame *lastDecodedFrame;
    
    int streamIndex;
    
    int bytesConsumedFromDecodedFrame;
    
    int readNextPacket;
    int endOfStream;
    int endOfAudio;
    
    int toConsume;
    
    int framesReadNow;
    
    if (data->totalFrames && data->framesRead >= data->totalFrames) {
        memset(outbuf, 0, samples_to_do * channels * sizeof(sample));
        return;
    }
    
    frameSize = data->channels * (data->bitsPerSample / 8);
    dataSize = 0;
    
    bytesToRead = samples_to_do * frameSize;
    bytesRead = 0;
    
    targetBuf = data->sampleBuffer;
    memset(targetBuf, 0, bytesToRead);
    
    formatCtx = data->formatCtx;
    codecCtx = data->codecCtx;
    lastReadPacket = data->lastReadPacket;
    lastDecodedFrame = data->lastDecodedFrame;
    
    streamIndex = data->streamIndex;
    
    bytesConsumedFromDecodedFrame = data->bytesConsumedFromDecodedFrame;
    
    readNextPacket = data->readNextPacket;
    endOfStream = data->endOfStream;
    endOfAudio = data->endOfAudio;

    while (bytesRead < bytesToRead) {
        int planeSize;
        int planar = av_sample_fmt_is_planar(codecCtx->sample_fmt);
        dataSize = av_samples_get_buffer_size(&planeSize, codecCtx->channels,
                                              lastDecodedFrame->nb_samples,
                                              codecCtx->sample_fmt, 1);
        
        if (dataSize < 0)
            dataSize = 0;
        
        while (readNextPacket && !endOfAudio) {
            if (!endOfStream) {
                av_packet_unref(lastReadPacket);
                if ((errcode = av_read_frame(formatCtx, lastReadPacket)) < 0) {
                    if (errcode == AVERROR_EOF) {
                        endOfStream = 1;
                    }
                    if (formatCtx->pb && formatCtx->pb->error)
                        break;
                }
                if (lastReadPacket->stream_index != streamIndex)
                    continue;
            }
            
            if ((errcode = avcodec_send_packet(codecCtx, endOfStream ? NULL : lastReadPacket)) < 0) {
                if (errcode != AVERROR(EAGAIN)) {
                    goto end;
                }
            }
            
            readNextPacket = 0;
        }
        
        if (dataSize <= bytesConsumedFromDecodedFrame) {
            if (endOfStream && endOfAudio)
                break;
            
            bytesConsumedFromDecodedFrame = 0;
            
            if ((errcode = avcodec_receive_frame(codecCtx, lastDecodedFrame)) < 0) {
                if (errcode == AVERROR_EOF) {
                    endOfAudio = 1;
                    break;
                }
                else if (errcode == AVERROR(EAGAIN)) {
                    readNextPacket = 1;
                    continue;
                }
                else {
                    goto end;
                }
            }
            
            dataSize = av_samples_get_buffer_size(&planeSize, codecCtx->channels, lastDecodedFrame->nb_samples, codecCtx->sample_fmt, 1);
            
            if (dataSize < 0)
                dataSize = 0;
        }
        
        toConsume = FFMIN((dataSize - bytesConsumedFromDecodedFrame), (bytesToRead - bytesRead));
        
        if (!planar || channels == 1) {
            memmove(targetBuf + bytesRead, (lastDecodedFrame->data[0] + bytesConsumedFromDecodedFrame), toConsume);
        }
        else {
            uint8_t * out = (uint8_t *) targetBuf + bytesRead;
            int bytesPerSample = data->bitsPerSample / 8;
            int bytesConsumedPerPlane = bytesConsumedFromDecodedFrame / channels;
            int toConsumePerPlane = toConsume / channels;
            int s, ch;
            for (s = 0; s < toConsumePerPlane; s += bytesPerSample) {
                for (ch = 0; ch < channels; ++ch) {
                    memcpy(out, lastDecodedFrame->extended_data[ch] + bytesConsumedPerPlane + s, bytesPerSample);
                    out += bytesPerSample;
                }
            }
        }
        
        bytesConsumedFromDecodedFrame += toConsume;
        bytesRead += toConsume;
    }
    
end:
    framesReadNow = bytesRead / frameSize;
    if (data->totalFrames && (data->framesRead + framesReadNow > data->totalFrames)) {
        framesReadNow = (int)(data->totalFrames - data->framesRead);
    }
    
    data->framesRead += framesReadNow;
    
    // Convert the audio
    convert_audio(outbuf, data->sampleBuffer, framesReadNow * channels, data->bitsPerSample, data->floatingPoint);
    
    // Output the state back to the structure
    data->bytesConsumedFromDecodedFrame = bytesConsumedFromDecodedFrame;
    data->readNextPacket = readNextPacket;
    data->endOfStream = endOfStream;
    data->endOfAudio = endOfAudio;
    
    // And pad if we ran short (end of file)
    if (framesReadNow < samples_to_do) {
        memset(outbuf + framesReadNow * channels, 0, sizeof(sample) * channels * (samples_to_do - framesReadNow));
    }
}

#endif
