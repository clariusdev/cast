package me.clarius.sdk.cast.example;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.Optional;

import me.clarius.sdk.Cast;
import me.clarius.sdk.ProbeInfo;
import me.clarius.sdk.UserFunction;
import me.clarius.sdk.cast.example.databinding.FragmentFirstBinding;

public class FirstFragment extends Fragment {

    private static final String TAG = "Cast";

    private FragmentFirstBinding binding;
    private CastService.CastBinder castBinder;
    private Long timestamp = 0L;

    /**
     * Defines callbacks for service binding, passed to bindService()
     */
    private final ServiceConnection castConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            Log.d(TAG, "service connected");
            // We've bound to our service, cast the IBinder now
            castBinder = (CastService.CastBinder) service;
            castBinder.getProcessedImage().observe(getViewLifecycleOwner(), binding.imageView::setImageBitmap);
            castBinder.getTimestamp().observe(getViewLifecycleOwner(), FirstFragment.this::setTimestamp);
            castBinder.getError().observe(getViewLifecycleOwner(), FirstFragment.this::showError);
            castBinder.getRawDataProgress().observe(getViewLifecycleOwner(), binding.rawDataDownloadProgressBar::setProgress);
        }

        @Override
        public void onServiceDisconnected(ComponentName component) {
            Log.d(TAG, "service disconnected");
            castBinder = null;
        }
    };

    private static String fromByteArray(final byte[] from) {
        return new String(from, StandardCharsets.UTF_8);
    }

    private static void askProbeInfo(Cast cast) {
        cast.getProbeInfo(info -> Log.d(TAG, "Probe info: " + info.map(FirstFragment::probeInfoToString).orElse("<none>")));
    }

    public static String probeInfoToString(final ProbeInfo info) {
        StringBuilder b = new StringBuilder();
        b.append("v").append(info.version).append(" elements: ").append(info.elements).append(" pitch: ").append(info.pitch).append(" radius: ").append(info.radius);
        return b.toString();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        binding = FragmentFirstBinding.inflate(inflater, container, false);
        return binding.getRoot();
    }

    public void onViewCreated(@NonNull View view, Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        binding.buttonConnect.setOnClickListener(v -> doConnect());
        binding.buttonRun.setOnClickListener(v -> toggleRun());
        binding.buttonDisconnect.setOnClickListener(v -> doDisconnect());
        binding.buttonRawData.setOnClickListener(v -> getRawData());
        binding.buttonCapture.setOnClickListener(v -> doCapture());
    }

    @Override
    public void onResume() {
        super.onResume();
        final Intent intent = this.requireActivity().getIntent();
        if (intent != null) {
            final Bundle extras = intent.getExtras();
            if (extras != null) {
                final Optional<String> probeSerial = Optional.ofNullable(extras.getByteArray("cus_probe_serial")).map(FirstFragment::fromByteArray);
                final Optional<String> ipAddress = Optional.ofNullable(extras.getByteArray("cus_ip_address")).map(FirstFragment::fromByteArray);
                final Optional<String> castPort = Optional.ofNullable(extras.getByteArray("cus_cast_port")).map(FirstFragment::fromByteArray);
                final Optional<String> networkId = Optional.ofNullable(extras.getByteArray("cus_network_id")).map(FirstFragment::fromByteArray);
                Log.d(TAG, "Received probe serial: " + probeSerial.orElse("<none>"));
                Log.d(TAG, "Received IP address: " + ipAddress.orElse("<none>"));
                Log.d(TAG, "Received cast port: " + castPort.orElse("<none>"));
                Log.d(TAG, "Received network ID: " + networkId.orElse("<none>"));
                ipAddress.ifPresent(s -> binding.ipAddress.setText(s));
                castPort.ifPresent(s -> binding.tcpPort.setText(s));
                networkId.ifPresent(s -> binding.networkId.setText(s));
            }
        }
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Context context = requireContext();
        Intent intent = new Intent(context, CastService.class);
        context.bindService(intent, castConnection, Context.BIND_AUTO_CREATE);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Context context = requireContext();
        context.unbindService(castConnection);
        castBinder = null;
    }

    private void toggleRun() {
        if (castBinder == null) {
            showError("Clarius Cast not initialized");
            return;
        }
        showMessage("Toggle run");
        castBinder.getCast().userFunction(UserFunction.Freeze, 0, result -> Log.d(TAG, "Freeze function result: " + result));
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        binding = null;
    }

    private void doConnect() {
        if (castBinder == null) {
            showError("Clarius Cast not initialized");
            return;
        }
        binding.ipAddressLayout.setError(null);
        binding.tcpPortLayout.setError(null);
        String ipAddress = String.valueOf(binding.ipAddress.getText());
        if (ipAddress.isEmpty()) {
            binding.ipAddressLayout.setError("Cannot be empty");
            return;
        }
        Optional<Integer> tcpPort = Utils.maybeInt(binding.tcpPort.getText());
        if (!tcpPort.isPresent()) {
            binding.tcpPortLayout.setError("Invalid number");
            return;
        }
        Optional<Long> networkId = Utils.maybeLong(binding.networkId.getText());
        showMessage("Connecting to " + ipAddress + ":" + tcpPort.get());
        castBinder.getCast().connect(ipAddress, tcpPort.get(), networkId, getCertificate(), (result, imagePort, imuPort, swRevMatch) -> {
            Log.d(TAG, "Connection result: " + result);
            if (result) {
                Log.d(TAG, "UDP stream will be on port " + imagePort);
                Log.d(TAG, "IMU stream will be on port " + imuPort);
                Log.d(TAG, "App software " + (swRevMatch ? "matches" : "does not match"));
                askProbeInfo(castBinder.getCast());
            }
        });
    }

    private void doDisconnect() {
        if (castBinder == null) {
            return;
        }
        castBinder.getCast().disconnect(result -> Log.d(TAG, "Disconnection result: " + result));
    }

    private void getRawData() {
        if (castBinder == null) {
            return;
        }
        showMessage("Requesting raw data");
        binding.rawDataDownloadProgressBar.setProgress(0);
        final Cast cast = castBinder.getCast();
        RawDataFile handle = new RawDataFile(cast);
        final int start = 0;
        final int end = 0;
        final boolean lzo = true;
        cast.requestRawData(start, end, lzo, handle::requestResultRetrieved);
    }

    private void doCapture() {
        if (castBinder == null || timestamp == 0L) {
            return;
        }
        showMessage("Starting image capture");
        castBinder.getCast().startCapture(timestamp, captureID -> {
            Log.d(TAG, "Start capture got ID: " + captureID);
            if (captureID < 0) {
                return;
            }
            castBinder.getCast().finishCapture(captureID, result -> {
                Log.d(TAG, "Finish capture got result: " + result);
            });
        });
    }

    private void setTimestamp(Long timestamp) {
        this.timestamp = timestamp;
    }

    private void showError(CharSequence text) {
        Log.e(TAG, "Error: " + text);
        Handler mainHandler = new Handler(Looper.getMainLooper());
        mainHandler.post(() -> Toast.makeText(requireContext(), text, Toast.LENGTH_SHORT).show());
    }

    private void showMessage(CharSequence text) {
        Log.d(TAG, (String) text);
        Handler mainHandler = new Handler(Looper.getMainLooper());
        mainHandler.post(() -> Toast.makeText(requireContext(), text, Toast.LENGTH_SHORT).show());
    }

    private String getCertificate() {
        return "research";
    }

    private class RawDataFile {
        final Cast cast;

        RawDataFile(Cast cast) {
            this.cast = cast;
        }

        void requestResultRetrieved(int result) {
            if (result > 0) {
                showMessage("Found raw data");
                cast.readRawData(this::rawDataRetrieved);
            } else {
                showMessage("Failed to request raw data, ensure raw data buffering is enabled and image is frozen");
            }
        }

        void rawDataRetrieved(int result, ByteBuffer data) {
            if (result > 0) {
                try {
                    showMessage("Saving raw data");
                    Uri uri = IOUtils.saveInDocuments(data, "cast_raw_data", requireContext());
                    showMessage("Saved raw data in file " + uri);
                } catch (IOException e) {
                    showError(e.toString());
                }
            } else {
                showError("Could not retrieve raw data");
            }
        }
    }
}
