package com.zhangteng.ffmpegdemo;

import android.view.Surface;

/**
 * Created by swing on 2019/4/12.
 */
public class VideoPlayer {
    private static VideoPlayer instance;

    private VideoPlayer() {
    }

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
}
