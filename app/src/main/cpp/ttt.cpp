#include <jni.h>
#include <cstdio>
#include <string>
#include <android/log.h>

extern "C" {
//编码
#include "libavcodec/avcodec.h"
//封装格式处理
#include "libavformat/avformat.h"
//像素处理
#include "libswscale/swscale.h"
//重采样
#include "libswresample/swresample.h"
}

#include "libyuv/libyuv.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <zconf.h>

#define FFLOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"ffmpeg",FORMAT,##__VA_ARGS__);
#define FFLOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"ffmpeg",FORMAT,##__VA_ARGS__);

extern "C" JNIEXPORT jint JNICALL
Java_com_zhangteng_ffmpegdemo_VideoPlayer_getStringC(JNIEnv *env, jobject jobj) {
    jclass cls = env->GetObjectClass(jobj);
    jfieldID fid = env->GetFieldID(cls, "key", "I");
    jint jstr = env->GetIntField(jobj, fid);
    printf("c++:%d", jstr);
    env->SetIntField(jobj, fid, jstr + 1);
    return jstr;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_zhangteng_ffmpegdemo_VideoPlayer_FFmpegTest(JNIEnv *env, jclass obj, jstring input,
                                                     jstring output) {
    //获取输入输出文件名
    const char *inputc = env->GetStringUTFChars((jstring) input, 0);
    const char *outputc = env->GetStringUTFChars((jstring) output, 0);
    //注册组件
    av_register_all();
    //打开文件
    AVFormatContext *avFormatContext = avformat_alloc_context();
    if (avformat_open_input(&avFormatContext, inputc, NULL, NULL) != 0) {
        FFLOGE("%s", "无法打开视频");
        return 0;
    }
    //找到流信息
    if (avformat_find_stream_info(avFormatContext, NULL) < 0) {
        FFLOGE("%s", "无法解析视频")
        return 1;
    }
    //查找视频流
    int i = 0;
    int video_stream_idx = -1;
    for (; i < avFormatContext->nb_streams; ++i) {
        if (avFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
        }
    }
    //获取解码器
    AVCodecContext *avCodecContext = avFormatContext->streams[video_stream_idx]->codec;
    AVCodec *avCodec = avcodec_find_decoder(avCodecContext->codec_id);
    if (avCodec == NULL) {
        FFLOGE("%s", "无法解码")
        return 2;
    }
    //打开解码器
    if (avcodec_open2(avCodecContext, avCodec, NULL) < 0) {
        FFLOGE("%s", "解码器出错")
        return 3;
    }
    //编码数据
    AVPacket *avPacket;
    av_init_packet(avPacket);
    //像素数据
    AVFrame *avFrame = av_frame_alloc();
    AVFrame *yuvAVFrame = av_frame_alloc();

    //只有指定了AVFrame的像素格式、画面大小才能真正分配内存
    //缓冲区分配内存
    uint8_t *out_buffer = (uint8_t *) av_malloc(
            avpicture_get_size(AV_PIX_FMT_YUV420P, avCodecContext->width, avCodecContext->height));
    //初始化缓冲区
    avpicture_fill((AVPicture *) yuvAVFrame, out_buffer, AV_PIX_FMT_YUV420P, avCodecContext->width,
                   avCodecContext->height);

    //用于转码（缩放）的参数，转之前的宽高，转之后的宽高，格式等
    struct SwsContext *sws_ctx = sws_getContext(avCodecContext->width, avCodecContext->height,
                                                avCodecContext->pix_fmt,
                                                avCodecContext->width, avCodecContext->height,
                                                AV_PIX_FMT_YUV420P,
                                                SWS_BILINEAR, NULL, NULL, NULL);
    int len, got_frame;
    int frame_count = 0;
    FILE *outFile = fopen(outputc, "wb");
    if (outFile == NULL) {
        FFLOGE("%s", "目标文件为NULL");
        return 4;
    }
    while (av_read_frame(avFormatContext, avPacket) >= 0) {
        //只要视频压缩数据（根据流的索引位置判断）
        if (avPacket->stream_index == video_stream_idx) {
            len = avcodec_decode_video2(avCodecContext, avFrame, &got_frame, avPacket);
            if (len < 0) {
                FFLOGE("%s", "解码错误");
                return 5;
            }
            if (got_frame) {
                sws_scale(sws_ctx, avFrame->data, avFrame->linesize, 0, avFrame->height,
                          yuvAVFrame->data, yuvAVFrame->linesize);
                int y_size = avCodecContext->width * avCodecContext->height;
                fwrite(yuvAVFrame->data[0], 1, y_size, outFile);
                fwrite(yuvAVFrame->data[1], 1, y_size / 4, outFile);
                fwrite(yuvAVFrame->data[2], 1, y_size / 4, outFile);
                frame_count++;
                FFLOGE("解码第%d帧", frame_count);
            }
        }
        av_free_packet(avPacket);
    }
    fclose(outFile);
    av_frame_free(&avFrame);
    avcodec_close(avCodecContext);
    avformat_close_input(&avFormatContext);
    return -1;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_zhangteng_ffmpegdemo_VideoPlayer_render(JNIEnv *env, jclass type, jstring input_,
                                                 jobject surface) {
    const char *input = env->GetStringUTFChars(input_, 0);
    //1.注册组件
    av_register_all();

    //封装格式上下文
    AVFormatContext *pFormatCtx = avformat_alloc_context();

    //2.打开输入视频文件
    if (avformat_open_input(&pFormatCtx, input, NULL, NULL) != 0) {
        FFLOGE("%s", "打开输入视频文件失败");
        return;
    }
    //3.获取视频信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        FFLOGE("%s", "获取视频信息失败");
        return;
    }

    //视频解码，需要找到视频对应的AVStream所在pFormatCtx->streams的索引位置
    int video_stream_idx = -1;
    int i = 0;
    for (; i < pFormatCtx->nb_streams; i++) {
        //根据类型判断，是否是视频流
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
            break;
        }
    }

    //4.获取视频解码器
    AVCodecContext *pCodeCtx = pFormatCtx->streams[video_stream_idx]->codec;
    AVCodec *pCodec = avcodec_find_decoder(pCodeCtx->codec_id);
    if (pCodec == NULL) {
        FFLOGE("%s", "无法解码");
        return;
    }

    //5.打开解码器
    if (avcodec_open2(pCodeCtx, pCodec, NULL) < 0) {
        FFLOGE("%s", "解码器无法打开");
        return;
    }

    //编码数据
    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));

    //像素数据（解码数据）
    AVFrame *yuv_frame = av_frame_alloc();
    AVFrame *rgb_frame = av_frame_alloc();

    //native绘制
    //窗体
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    //绘制时的缓冲区
    ANativeWindow_Buffer outBuffer;

    int len, got_frame, framecount = 0;
    //6.一阵一阵读取压缩的视频数据AVPacket
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        //解码AVPacket->AVFrame
        len = avcodec_decode_video2(pCodeCtx, yuv_frame, &got_frame, packet);

        //Zero if no frame could be decompressed
        //非零，正在解码
        if (got_frame) {
            FFLOGI("解码%d帧", framecount++);
            //lock
            //设置缓冲区的属性（宽、高、像素格式）
            ANativeWindow_setBuffersGeometry(nativeWindow, pCodeCtx->width, pCodeCtx->height,
                                             WINDOW_FORMAT_RGBA_8888);
            ANativeWindow_lock(nativeWindow, &outBuffer, NULL);

            //设置rgb_frame的属性（像素格式、宽高）和缓冲区
            //rgb_frame缓冲区与outBuffer.bits是同一块内存
            avpicture_fill((AVPicture *) rgb_frame, static_cast<const uint8_t *>(outBuffer.bits),
                           AV_PIX_FMT_RGBA,
                           pCodeCtx->width, pCodeCtx->height);

            //YUV->RGBA_8888
            libyuv::I420ToARGB(yuv_frame->data[0], yuv_frame->linesize[0],
                               yuv_frame->data[2], yuv_frame->linesize[2],
                               yuv_frame->data[1], yuv_frame->linesize[1],
                               rgb_frame->data[0], rgb_frame->linesize[0],
                               pCodeCtx->width, pCodeCtx->height);

            //unlock
            ANativeWindow_unlockAndPost(nativeWindow);

            usleep(1000 * 16);

        }

        av_free_packet(packet);
    }

    ANativeWindow_release(nativeWindow);
    av_frame_free(&yuv_frame);
    avcodec_close(pCodeCtx);
    avformat_free_context(pFormatCtx);

    env->ReleaseStringUTFChars(input_, input);
}
#define MAX_AUDIO_FRME_SIZE 48000 * 4
extern "C"
JNIEXPORT void JNICALL Java_com_zhangteng_ffmpegdemo_VideoPlayer_sound
        (JNIEnv *env, jobject jobj, jstring input_jstr, jstring output_jstr){
    const char* input_cstr = env->GetStringUTFChars(input_jstr,NULL);
    const char* output_cstr = env->GetStringUTFChars(output_jstr,NULL);
    FFLOGI("%s","sound");
    //注册组件
    av_register_all();
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    //打开音频文件
    if(avformat_open_input(&pFormatCtx,input_cstr,NULL,NULL) != 0){
        FFLOGI("%s","无法打开音频文件");
        return;
    }
    //获取输入文件信息
    if(avformat_find_stream_info(pFormatCtx,NULL) < 0){
        FFLOGI("%s","无法获取输入文件信息");
        return;
    }
    //获取音频流索引位置
    int i = 0, audio_stream_idx = -1;
    for(; i < pFormatCtx->nb_streams;i++){
        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
            audio_stream_idx = i;
            break;
        }
    }

    //获取解码器
    AVCodecContext *codecCtx = pFormatCtx->streams[audio_stream_idx]->codec;
    AVCodec *codec = avcodec_find_decoder(codecCtx->codec_id);
    if(codec == NULL){
        FFLOGI("%s","无法获取解码器");
        return;
    }
    //打开解码器
    if(avcodec_open2(codecCtx,codec,NULL) < 0){
        FFLOGI("%s","无法打开解码器");
        return;
    }
    //压缩数据
    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    //解压缩数据
    AVFrame *frame = av_frame_alloc();
    //frame->16bit 44100 PCM 统一音频采样格式与采样率
    SwrContext *swrCtx = swr_alloc();

    //重采样设置参数-------------start
    //输入的采样格式
    enum AVSampleFormat in_sample_fmt = codecCtx->sample_fmt;
    //输出采样格式16bit PCM
    enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    //输入采样率
    int in_sample_rate = codecCtx->sample_rate;
    //输出采样率
    int out_sample_rate = 44100;
    //获取输入的声道布局
    //根据声道个数获取默认的声道布局（2个声道，默认立体声stereo）
    //av_get_default_channel_layout(codecCtx->channels);
    uint64_t in_ch_layout = codecCtx->channel_layout;
    //输出的声道布局（立体声）
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;

    swr_alloc_set_opts(swrCtx,
                       out_ch_layout,out_sample_fmt,out_sample_rate,
                       in_ch_layout,in_sample_fmt,in_sample_rate,
                       0, NULL);
    swr_init(swrCtx);

    //输出的声道个数
    int out_channel_nb = av_get_channel_layout_nb_channels(out_ch_layout);

    //重采样设置参数-------------end

    //16bit 44100 PCM 数据
    uint8_t *out_buffer = (uint8_t *)av_malloc(MAX_AUDIO_FRME_SIZE);

    FILE *fp_pcm = fopen(output_cstr,"wb");

    int got_frame = 0,index = 0, ret;
    //不断读取压缩数据
    while(av_read_frame(pFormatCtx,packet) >= 0){
        //解码
        ret = avcodec_decode_audio4(codecCtx,frame,&got_frame,packet);

        if(ret < 0){
            FFLOGI("%s","解码完成");
        }
        //解码一帧成功
        if(got_frame > 0){
            FFLOGI("解码：%d",index++);
            swr_convert(swrCtx, &out_buffer, MAX_AUDIO_FRME_SIZE,frame->data,frame->nb_samples);
            //获取sample的size
            int out_buffer_size = av_samples_get_buffer_size(NULL, out_channel_nb,
                                                             frame->nb_samples, out_sample_fmt, 1);
            fwrite(out_buffer,1,out_buffer_size,fp_pcm);
        }

        av_free_packet(packet);
    }

    fclose(fp_pcm);
    av_frame_free(&frame);
    av_free(out_buffer);

    swr_free(&swrCtx);
    avcodec_close(codecCtx);
    avformat_close_input(&pFormatCtx);

    env->ReleaseStringUTFChars(input_jstr,input_cstr);
    env->ReleaseStringUTFChars(output_jstr,output_cstr);

}

extern "C"
JNIEXPORT void JNICALL Java_com_zhangteng_ffmpegdemo_VideoPlayer_soundAudioTrack
        (JNIEnv *env, jobject obj, jstring input_jstr, jstring output_jstr) {
    const char *input_cstr = env->GetStringUTFChars(input_jstr, NULL);
    const char *output_cstr = env->GetStringUTFChars(output_jstr, NULL);
    FFLOGI("%s", "sound");
    //注册组件
    av_register_all();
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    //打开音频文件
    if (avformat_open_input(&pFormatCtx, input_cstr, NULL, NULL) != 0) {
        FFLOGI("%s", "无法打开音频文件");
        return;
    }
    //获取输入文件信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        FFLOGI("%s", "无法获取输入文件信息");
        return;
    }
    //获取音频流索引位置
    int i = 0, audio_stream_idx = -1;
    for (; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_idx = i;
            break;
        }
    }

    //获取解码器
    AVCodecContext *codecCtx = pFormatCtx->streams[audio_stream_idx]->codec;
    AVCodec *codec = avcodec_find_decoder(codecCtx->codec_id);
    if (codec == NULL) {
        FFLOGI("%s", "无法获取解码器");
        return;
    }
    //打开解码器
    if (avcodec_open2(codecCtx, codec, NULL) < 0) {
        FFLOGI("%s", "无法打开解码器");
        return;
    }
    //压缩数据
    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));
    //解压缩数据
    AVFrame *frame = av_frame_alloc();
    //frame->16bit 44100 PCM 统一音频采样格式与采样率
    SwrContext *swrCtx = swr_alloc();

    //重采样设置参数-------------start
    //输入的采样格式
    enum AVSampleFormat in_sample_fmt = codecCtx->sample_fmt;
    //输出采样格式16bit PCM
    enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    //输入采样率
    int in_sample_rate = codecCtx->sample_rate;
    //输出采样率
    int out_sample_rate = in_sample_rate;
    //获取输入的声道布局
    //根据声道个数获取默认的声道布局（2个声道，默认立体声stereo）
    //av_get_default_channel_layout(codecCtx->channels);
    uint64_t in_ch_layout = codecCtx->channel_layout;
    //输出的声道布局（立体声）
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;

    swr_alloc_set_opts(swrCtx,
                       out_ch_layout, out_sample_fmt, out_sample_rate,
                       in_ch_layout, in_sample_fmt, in_sample_rate,
                       0, NULL);
    swr_init(swrCtx);

    //输出的声道个数
    int out_channel_nb = av_get_channel_layout_nb_channels(out_ch_layout);

    //重采样设置参数-------------end

    //JNI begin------------------
    //JasonPlayer
    jclass player_class = env->GetObjectClass(obj);

    //AudioTrack对象
    jmethodID create_audio_track_mid = env->GetMethodID(player_class, "createAudioTrack",
                                                        "()Landroid/media/AudioTrack;");
    jobject audio_track = env->CallObjectMethod(obj, create_audio_track_mid, out_sample_rate,
                                                out_channel_nb);

    //调用AudioTrack.play方法
    jclass audio_track_class = env->GetObjectClass(audio_track);
    jmethodID audio_track_play_mid = env->GetMethodID(audio_track_class, "play", "()V");
    env->CallVoidMethod(audio_track, audio_track_play_mid);

    //AudioTrack.write
    jmethodID audio_track_write_mid = env->GetMethodID(audio_track_class, "write", "([BII)I");

    //JNI end------------------
    FILE *fp_pcm = fopen(output_cstr, "wb");

    //16bit 44100 PCM 数据
    uint8_t *out_buffer = (uint8_t *) av_malloc(MAX_AUDIO_FRME_SIZE);

    int got_frame = 0, index = 0, ret;
    //不断读取压缩数据
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        //解码音频类型的Packet
        if (packet->stream_index == audio_stream_idx) {
            //解码
            ret = avcodec_decode_audio4(codecCtx, frame, &got_frame, packet);

            if (ret < 0) {
                FFLOGI("%s", "解码完成");
            }
            //解码一帧成功
            if (got_frame > 0) {
                FFLOGI("解码：%d", index++);
                swr_convert(swrCtx, &out_buffer, MAX_AUDIO_FRME_SIZE,
                            (const uint8_t **) frame->data, frame->nb_samples);
                //获取sample的size
                int out_buffer_size = av_samples_get_buffer_size(NULL, out_channel_nb,
                                                                 frame->nb_samples, out_sample_fmt,
                                                                 1);
                fwrite(out_buffer, 1, out_buffer_size, fp_pcm);

                //out_buffer缓冲区数据，转成byte数组
                jbyteArray audio_sample_array = env->NewByteArray(out_buffer_size);
                jbyte *sample_bytep = env->GetByteArrayElements(audio_sample_array, NULL);
                //out_buffer的数据复制到sampe_bytep
                memcpy(sample_bytep, out_buffer, out_buffer_size);
                //同步
                env->ReleaseByteArrayElements(audio_sample_array, sample_bytep, 0);

                //AudioTrack.write PCM数据
                env->CallIntMethod(audio_track, audio_track_write_mid,
                                   audio_sample_array, 0, out_buffer_size);
                //释放局部引用
                env->DeleteLocalRef(audio_sample_array);
                usleep(1000 * 16);
            }
        }

        av_free_packet(packet);
    }

    av_frame_free(&frame);
    av_free(out_buffer);

    swr_free(&swrCtx);
    avcodec_close(codecCtx);
    avformat_close_input(&pFormatCtx);

    env->ReleaseStringUTFChars(input_jstr, input_cstr);
    env->ReleaseStringUTFChars(output_jstr, output_cstr);

}
