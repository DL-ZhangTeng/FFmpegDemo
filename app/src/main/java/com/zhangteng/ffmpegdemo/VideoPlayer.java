package com.zhangteng.ffmpegdemo;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.view.Surface;

/**
 * Created by swing on 2019/4/12.
 */
public class VideoPlayer {
    private static VideoPlayer instance;

//    private VideoPlayer() {
//    }

    public static VideoPlayer getInstance() {
        if (instance == null) {
            synchronized (VideoPlayer.class) {
                if (instance == null) {
                    instance = new VideoPlayer();
                }
            }
        }
        return instance;
    }

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("ttt");
        System.loadLibrary("dn_player");
        System.loadLibrary("avcodec");
        System.loadLibrary("avfilter");
        System.loadLibrary("avformat");
        System.loadLibrary("avutil");
        System.loadLibrary("swresample");
        System.loadLibrary("swscale");
        System.loadLibrary("fdk-aac");
        System.loadLibrary("yuv");
    }

    public int key = 100;

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native int getStringC();

    public static native int FFmpegTest(String input, String output);

    public static native void render(String input, Surface surface);

    public native void sound(String input, String output);

    public native void soundAudioTrack(String input, String output);

    public native void player(String input, Surface surface);

    /**
     * 创建一个AudioTrac对象，用于播放
     *
     * @return
     */
    public AudioTrack createAudioTrack() {
        int sampleRateInHz = 44100;
        int audioFormat = AudioFormat.ENCODING_PCM_16BIT;
        //声道布局
        int channelConfig = android.media.AudioFormat.CHANNEL_OUT_STEREO;

        int bufferSizeInBytes = AudioTrack.getMinBufferSize(sampleRateInHz, channelConfig, audioFormat);
        AudioTrack audioTrack = new AudioTrack(
                AudioManager.STREAM_MUSIC,
                sampleRateInHz, channelConfig,
                audioFormat,
                bufferSizeInBytes, AudioTrack.MODE_STREAM);
        //播放
        //audioTrack.play();
        //写入PCM
        //audioTrack.write(byte[]buffer);
        return audioTrack;
    }
}
