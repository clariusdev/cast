package me.clarius.sdk.cast.example;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import java.nio.ByteBuffer;
import java.util.concurrent.Executor;

import me.clarius.sdk.ImageFormat;
import me.clarius.sdk.ProcessedImageInfo;

/**
 * Convert images in a separate thread to avoid blocking the producer (the SDK)
 */

public class ImageConverter {
    private final Executor executor;
    private final Callback callback;

    ImageConverter(Executor executor, Callback callback) {
        this.executor = executor;
        this.callback = callback;
    }

    public void convertImage(ByteBuffer buffer, ProcessedImageInfo info) {
        executor.execute(() -> {
            try {
                Bitmap bitmap = convert(buffer, info);
                callback.onResult(bitmap, info.tm);
            } catch (Exception e) {
                callback.onError(e);
            }
        });
    }

    private Bitmap convert(ByteBuffer buffer, ProcessedImageInfo info) {
        boolean isCompressed = info.format != ImageFormat.Uncompressed;
        Bitmap bitmap;
        if (isCompressed) {
            if (buffer.hasArray()) {
                byte[] bytes = buffer.array();
                int offset = buffer.arrayOffset();
                int length = info.imageSize;
                assert offset + length < bytes.length;
                bitmap = BitmapFactory.decodeByteArray(bytes, offset, length);
            } else {
                byte[] bytes = new byte[buffer.capacity()];
                buffer.get(bytes);
                bitmap = BitmapFactory.decodeByteArray(bytes, 0, bytes.length);
            }
        } else {
            bitmap = Bitmap.createBitmap(info.width, info.height, Bitmap.Config.ARGB_8888);
            bitmap.copyPixelsFromBuffer(buffer);
        }
        if (bitmap == null)
            throw new AssertionError("bad image data");
        return bitmap;
    }

    interface Callback {
        void onResult(Bitmap bitmap, long timestamp);

        void onError(Exception e);
    }
}
