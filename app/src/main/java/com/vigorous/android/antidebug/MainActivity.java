package com.vigorous.android.antidebug;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;

import com.vigorous.android.antidebug.nativeInterface.IJniInterface;

public class MainActivity extends AppCompatActivity {



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Log.e("antidebug",""+IJniInterface.iJNIE());
    }
}
