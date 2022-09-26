package me.clarius.sdk.cast.example;

import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;
import androidx.navigation.fragment.NavHostFragment;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import me.clarius.sdk.UserFunction;
import me.clarius.sdk.cast.example.databinding.FragmentFirstBinding;

public class FirstFragment extends Fragment {

    private static final String TAG = "Cast";
    private final ExecutorService executorService = Executors.newFixedThreadPool(1);

    private FragmentFirstBinding binding;
    private CastWrapper wrapper;
    private CastViewModel viewModel;

    @Override
    public View onCreateView(
            LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState
    ) {
        binding = FragmentFirstBinding.inflate(inflater, container, false);
        return binding.getRoot();
    }

    public void onViewCreated(@NonNull View view, Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        viewModel = new ViewModelProvider(this).get(CastViewModel.class);

        wrapper = new CastWrapper(requireContext(), executorService, viewModel);

        viewModel.getProcessedImage().observe(getViewLifecycleOwner(), binding.imageView::setImageBitmap);
        viewModel.getError().observe(getViewLifecycleOwner(), this::showError);

        binding.buttonBluetooth.setOnClickListener(v -> NavHostFragment.findNavController(FirstFragment.this)
                .navigate(R.id.action_FirstFragment_to_SecondFragment));
        binding.buttonConnect.setOnClickListener(v -> doConnect());
        binding.buttonRun.setOnClickListener(v -> toggleRun());
        binding.buttonAskState.setOnClickListener(v -> wrapper.askState());
        binding.buttonDisconnect.setOnClickListener(v -> doDisconnect());
    }

    private void toggleRun() {
        if (wrapper == null) {
            showError("Clarius Cast not initialized");
            return;
        }
        Log.d(TAG, "run freeze");
        wrapper.getCast().userFunction(UserFunction.Freeze, 0, UserFunction.Freeze);
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        if (wrapper != null) {
            wrapper.getCast().disconnect();
            wrapper.getCast().release();
            wrapper = null;
        }
        binding = null;
    }

    private void updateImage(Bitmap bitmap) {
        binding.imageView.setImageBitmap(bitmap);
    }

    private void doConnect() {
        if (wrapper == null) {
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
        int tcpPort;
        try {
            tcpPort = Integer.parseInt(String.valueOf(binding.tcpPort.getText()));
        } catch (RuntimeException e) {
            binding.tcpPortLayout.setError("Invalid number");
            return;
        }
        Log.d(TAG, "Connecting to " + ipAddress + ":" + tcpPort);
        wrapper.getCast().connect(ipAddress, tcpPort, getCertificate());
    }

    private void doDisconnect() {
        if (wrapper == null) {
            return;
        }
        wrapper.getCast().disconnect();
    }

    private void showError(CharSequence text) {
        Log.e(TAG, "Error: " + text);
        Handler mainHandler = new Handler(Looper.getMainLooper());
        mainHandler.post(() -> Toast.makeText(requireContext(), text, Toast.LENGTH_SHORT).show());
    }

    private String getCertificate() {
        return "research";
    }
}
