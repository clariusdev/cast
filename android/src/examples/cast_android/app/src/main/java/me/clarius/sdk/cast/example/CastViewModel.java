package me.clarius.sdk.cast.example;

import android.graphics.Bitmap;

import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

/**
 * View model for data produced by the SDK
 * <p>
 * Read more about live data: https://developer.android.com/topic/libraries/architecture/livedata.
 */

public class CastViewModel extends ViewModel {

    private final MutableLiveData<Bitmap> processedImage = new MutableLiveData<>();
    private final MutableLiveData<String> error = new MutableLiveData<>();

    public MutableLiveData<Bitmap> getProcessedImage() {
        return processedImage;
    }

    public MutableLiveData<String> getError() {
        return error;
    }
}
