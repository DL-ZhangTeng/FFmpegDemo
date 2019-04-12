package com.zhangteng.ffmpegdemo;

import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import com.zhangteng.ffmpegdemo.widget.VideoView;

import java.io.File;

import static com.zhangteng.ffmpegdemo.VideoPlayer.FFmpegTest;
import static com.zhangteng.ffmpegdemo.VideoPlayer.render;

public class MainActivity extends AppCompatActivity {

    private VideoView surfaceView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        VideoPlayer.getInstance().getStringC();
        tv.setText("" + VideoPlayer.getInstance().key);

        surfaceView = findViewById(R.id.surfaceview);

    }

    public void onPlay(View view) {
        File file = new File(Environment.getExternalStorageDirectory(), "aa.mp4");
        if (file.exists()) {
            String input = file.getAbsolutePath();
            render(input, surfaceView.getHolder().getSurface());
        }
    }

    public void ffmpegtest() {
        new Thread() {
            @Override
            public void run() {
                super.run();
                try {
                    sleep(3000);
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
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

            }
        }.start();
    }
}
