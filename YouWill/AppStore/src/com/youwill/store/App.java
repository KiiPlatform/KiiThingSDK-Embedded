package com.youwill.store;

import android.app.Application;

import com.kii.cloud.storage.Kii;

/**
 * Created by Evan on 14/10/19.
 */
public class App extends Application {
    @Override
    public void onCreate() {
        super.onCreate();
        Kii.initialize("c99e04f1", "3ebdc0472c0c705bc50eaf1756061b8b", Kii.Site.CN);
    }
}
