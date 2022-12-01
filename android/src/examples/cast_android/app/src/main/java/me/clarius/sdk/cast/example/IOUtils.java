package me.clarius.sdk.cast.example;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.net.Uri;
import android.os.Build;
import android.provider.MediaStore;

import androidx.annotation.RequiresApi;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.function.BiConsumer;

public class IOUtils {

    /**
     * Copy the entire content of the input stream into the output with a progression callback.
     * <p>
     * Sadly Android doesn't seem to be able to copy a file to a stream without an intermediate buffer.
     * Java 9 has inputStream.transferTo()... It should be added in Android 13.
     *
     * @param from       input stream (read from)
     * @param to         output stream (write to)
     * @param progressFn progress callback, can be null
     * @return the number of bytes transferred.
     */
    public static long copyBuffer(
            final ByteBuffer from,
            final OutputStream to,
            final BiConsumer<Integer, Integer> progressFn
    ) throws IOException {
        final byte[] buffer = new byte[4 * 1024];
        final int capacity = from.capacity();
        int copied = 0;
        int remaining;
        while ((remaining = from.remaining()) > 0) {
            final int N = Math.min(remaining, buffer.length);
            from.get(buffer, 0, N);
            to.write(buffer, 0, N);
            copied += N;
            if (null != progressFn) {
                progressFn.accept(copied, capacity);
            }
        }
        return copied;
    }

    @RequiresApi(api = Build.VERSION_CODES.Q)
    private static Uri doSaveInDownloads(ByteBuffer buffer, Context context, BiConsumer<Integer, Integer> progressFn) throws IOException {
        final DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyyMMdd_HHmmss");
        final String fileName = "clarius_" + LocalDateTime.now().format(formatter) + ".tar";
        final ContentValues contentValues = new ContentValues();
        contentValues.put(MediaStore.Downloads.DISPLAY_NAME, fileName);
        contentValues.put(MediaStore.Downloads.MIME_TYPE, "application/x-tar");
        contentValues.put(MediaStore.Downloads.RELATIVE_PATH, "Download/Clarius");
        contentValues.put(MediaStore.Downloads.IS_DOWNLOAD, true);
        contentValues.put(MediaStore.Downloads.IS_PENDING, true);

        final ContentResolver contentResolver = context.getContentResolver();
        final Uri uri = MediaStore.Downloads.getContentUri(MediaStore.VOLUME_EXTERNAL_PRIMARY);
        final Uri itemUri = contentResolver.insert(uri, contentValues);
        if (itemUri == null) {
            throw new IOException("Failed to create the raw data file in the Downloads folder");
        }

        try (OutputStream dest = contentResolver.openOutputStream(itemUri)) {
            copyBuffer(buffer, dest, progressFn);
        }

        contentValues.put(MediaStore.Downloads.IS_PENDING, false);
        contentResolver.update(itemUri, contentValues, null, null);

        return itemUri;
    }

    /**
     * Save the given byte buffer in the Downloads folder.
     * <p>
     * NOTE: this method uses the MediaStore.Downloads.getContentUri() API which is only available on Android 10 and later.
     * Calling this method on older Android will raise an exception.
     *
     * @param buffer     the byte buffer to save.
     * @param context    the context to retrieve the Downloads folder.
     * @param progressFn function to signal copy progress.
     * @return the saved file location.
     * @throws IOException
     */
    public static Uri saveInDownloads(ByteBuffer buffer, Context context, BiConsumer<Integer, Integer> progressFn) throws IOException {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            return doSaveInDownloads(buffer, context, progressFn);
        } else {
            throw new IOException("Saving only supported on Android 10 and later (API Q)");
        }
    }
}
