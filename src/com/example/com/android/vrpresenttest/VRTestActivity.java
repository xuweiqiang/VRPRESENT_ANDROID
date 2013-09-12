package com.example.com.android.vrpresenttest;

import com.example.com.android.vrpresenttest.GL2JNIView;

import android.os.Bundle;
import android.app.Activity;
import android.view.Menu;
//import android.view.MenuItem;
//import android.support.v4.app.NavUtils;

public class VRTestActivity extends Activity {
    GL2JNIView mView;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mView = new GL2JNIView(getApplication());
        setContentView(mView);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.activity_vrtest, menu);
        return true;
    }

    @Override protected void onPause() {
        super.onPause();
        mView.onPause();
    }

    @Override protected void onResume() {
        super.onResume();
        mView.onResume();
    }

}
