package com.nordtronics.hermestest;

import android.os.Bundle;
import android.widget.Button;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity {
    private TextView textView;
    private Button button;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        textView = findViewById(R.id.text_hello);
        button = findViewById(R.id.change_button);

        button.setOnClickListener(v -> {
            textView.setText("Button was tapped!");
        });
    }
}