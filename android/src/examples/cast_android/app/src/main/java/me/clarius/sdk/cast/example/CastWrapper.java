package me.clarius.sdk.cast.example;

import android.content.Context;
import android.graphics.Bitmap;
import android.util.Log;

import java.nio.ByteBuffer;
import java.util.Optional;
import java.util.concurrent.Executor;

import me.clarius.sdk.Button;
import me.clarius.sdk.Cast;
import me.clarius.sdk.Platform;
import me.clarius.sdk.PosInfo;
import me.clarius.sdk.ProbeInfo;
import me.clarius.sdk.ProcessedImageInfo;
import me.clarius.sdk.RawImageInfo;
import me.clarius.sdk.SpectralImageInfo;
import me.clarius.sdk.UserFunction;

class CastWrapper {
    private static final String TAG = "Cast";
    private static final String NONE = "<none>";
    private final Cast cast;
    private final CastViewModel viewModel;
    private final ImageConverter converter;
    private final Cast.Listener listener = new Cast.Listener() {

        @Override
        public void error(String e) {
            viewModel.getError().postValue(e);
        }

        @Override
        public void freeze(boolean frozen) {
            Log.d(TAG, "Freeze: " + frozen);
        }

        @Override
        public void initializationResult(boolean result) {
            Log.d(TAG, "Initialization result: " + result);
            if (result) {
                cast.getFirmwareVersion(Platform.V1, Platform.V1);
                cast.getFirmwareVersion(Platform.HD, Platform.HD);
                cast.getFirmwareVersion(Platform.HD3, Platform.HD3);
            }
        }

        @Override
        public void connectionResult(boolean result) {
            Log.d(TAG, "Connection result: " + result);
        }

        @Override
        public void disconnectionResult(boolean result) {
            Log.d(TAG, "Disconnection result: " + result);
        }

        @Override
        public void newProcessedImage(ByteBuffer buffer, ProcessedImageInfo info, PosInfo[] pos) {
            converter.convertImage(buffer, info);
        }

        @Override
        public void newRawImageFn(ByteBuffer buffer, RawImageInfo info, PosInfo[] pos) {
        }

        @Override
        public void newSpectralImageFn(ByteBuffer buffer, SpectralImageInfo info) {
        }

        @Override
        public void firmwareVersionRetrieved(Optional<String> version, Object user) {
            Log.d(TAG, "Firmware version: " + version.orElse(NONE)
                    + " (user param: " + user + ")");
        }

        @Override
        public void probeInfoRetrieved(Optional<ProbeInfo> info, Object user) {
            Log.d(TAG, "Probe info: " + info.map(this::fromProbeInfo).orElse(NONE)
                    + " (user param: " + (String)user + ")");
        }

        @Override
        public void userFunctionResult(boolean result, Object user) {
            Log.d(TAG, "User function result: " + result
                    + " (user param: " + user + ")");
        }

        @Override
        public void buttonPressed(Button button, int count) {
            Log.d(TAG, "Button '" + button + " pressed " + count + " time(s)");
        }

        private String fromProbeInfo(final ProbeInfo info) {
            StringBuilder b = new StringBuilder();
            b.append("v").append(info.version).append(" elements: ").append(info.elements)
                    .append(" pitch: ").append(info.pitch)
                    .append(" radius: ").append(info.radius);
            return b.toString();
        }
    };

    public CastWrapper(Context context, Executor executor, CastViewModel viewModel) {
        this.viewModel = viewModel;
        this.converter = new ImageConverter(executor, new ImageConverter.Callback() {
            @Override
            public void onResult(Bitmap bitmap) {
                viewModel.getProcessedImage().postValue(bitmap);
            }

            @Override
            public void onError(Exception e) {
                viewModel.getError().postValue(e.toString());
            }
        });
        cast = new Cast(context, listener);
        cast.initialize(getCertDir(context));
    }

    private static String getCertDir(Context context) {
        return context.getDir("cert", Context.MODE_PRIVATE).toString();
    }

    public Cast getCast() {
        return cast;
    }

    public void askState() {
        cast.getProbeInfo("asking probe info");
    }
}
