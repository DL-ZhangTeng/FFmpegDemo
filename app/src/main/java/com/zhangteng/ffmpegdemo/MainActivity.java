package com.zhangteng.ffmpegdemo;

import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.TextView;

import java.io.File;

public class MainActivity extends AppCompatActivity {

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
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        getStringC();
        tv.setText("" + key);
        new Thread() {
            @Override
            public void run() {
                super.run();
                try {
                    sleep(3000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                File file = new File(Environment.getExternalStorageDirectory(), "aa.mp4");
                File outFile = new File(Environment.getExternalStorageDirectory(), "bb.yuv");
                if (file.exists()) {
                    String input = file.getAbsolutePath();
                    Log.e("file", input);
                    String out = outFile.getAbsolutePath();
                    Log.e("file", out);
                    FFmpegTest(input, out);
                } else {
                    Log.e("file", "file no exists :" + file.getAbsolutePath());
                }
            }
        }.start();
    }


    public int key = 100;

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native int getStringC();

    public static native int JniCppAdd(int a, int b);

    public static native int JniCppSub(int a, int b);

    public static native int FFmpegTest(String input, String output);
}
