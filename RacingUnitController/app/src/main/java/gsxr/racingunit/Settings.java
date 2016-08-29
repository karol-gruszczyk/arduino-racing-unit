package gsxr.racingunit;

import android.content.Intent;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.ProgressBar;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;

import java.util.Locale;


public class Settings extends AppCompatActivity {
    BluetoothContext btContext;
    Switch wcEnabledSwitch, qsEnabledSwitch;
    TextView gyroTextView, wcAngleTextView, wcKillTimeTextView, lcRpmTextView, lcKillTimeTextView,
            lcWorkingTimeTextView, rpmTextView, qsMinRpmTextView, qsMaxRpmTextView,
            qsSensorTextView, qsSensitivityTextView;
    SeekBar wcAngleSeekBar, wcKillTimeSeekBar, lcRpmSeekBar, lcKillTimeSeekBar,
            lcWorkingTimeSeekBar, qsMinRpmSeekBar, qsMaxRpmSeekBar, qsSensitivitySeekBar;
    ProgressBar qsSensorProgressBar, rpmProgressBar;
    Button gyroButton, qsKillTimesButton, saveButton, backButton;

    private int maxRpm = 100;
    private int qsSensorMax = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);

        btContext = MainScreen.btContext;

        gyroTextView = (TextView)findViewById(R.id.gyroTextView);
        gyroButton = (Button)findViewById(R.id.gyroButton);
        gyroButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String command = "CAL_GYRO";
                btContext.sendCommand(command);
            }
        });

        wcEnabledSwitch = (Switch)findViewById(R.id.wcEnabledSwitch);
        wcEnabledSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean isChecked) {
                String command = "SWC:" + (isChecked ? "ON" : "OFF");
                btContext.sendCommand(command);
            }
        });

        wcAngleTextView = (TextView)findViewById(R.id.wcAngleTextView);
        wcAngleSeekBar = (SeekBar)findViewById(R.id.wcAngleSeekBar);
        wcAngleSeekBar.setMax(60);
        wcAngleSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                progress += 10;
                wcAngleTextView.setText(String.format(Locale.getDefault(),
                        "Max angle: %d°", progress));
                if (fromUser) {
                    String command = "SWC_ANGLE:" + progress;
                    btContext.sendCommand(command);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        wcKillTimeTextView = (TextView)findViewById(R.id.wcKillTimeTextView);
        wcKillTimeSeekBar = (SeekBar)findViewById(R.id.wcKillTimeSeekBar);
        wcKillTimeSeekBar.setMax(45);
        wcKillTimeSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                progress = progress * 10 + 50;
                wcKillTimeTextView.setText(String.format(Locale.getDefault(),
                        "Kill time: %d ms", progress));
                if (fromUser) {
                    String command = "SWC_KT:" + progress;
                    btContext.sendCommand(command);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        lcRpmTextView = (TextView)findViewById(R.id.lcRpmTextView);
        lcKillTimeTextView = (TextView)findViewById(R.id.lcKillTimeTextView);
        lcRpmSeekBar = (SeekBar)findViewById(R.id.lcRpmSeekBar);
        lcRpmSeekBar.setMax(32);
        lcRpmSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                progress = progress * 250 + 2000;
                lcRpmTextView.setText(String.format(Locale.getDefault(),
                        "Cut-off RPM: %d", progress));
                if (fromUser) {
                    String command = "SLC_RPM:" + progress;
                    btContext.sendCommand(command);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        lcKillTimeSeekBar = (SeekBar)findViewById(R.id.lcKillTimeSeekBar);
        lcKillTimeSeekBar.setMax(45);
        lcKillTimeSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                progress = progress * 10 + 50;
                lcKillTimeTextView.setText(String.format(Locale.getDefault(),
                        "Kill time: %d ms", progress));
                if (fromUser) {
                    String command = "SLC_KT:" + progress;
                    btContext.sendCommand(command);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        lcWorkingTimeTextView = (TextView)findViewById(R.id.lcWorkingTimeTextView);
        lcWorkingTimeSeekBar = (SeekBar)findViewById(R.id.lcWorkingTimeSeekBar);
        lcWorkingTimeSeekBar.setMax(90);
        lcWorkingTimeSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                progress = progress * 100 + 1000;
                lcWorkingTimeTextView.setText(String.format(Locale.getDefault(),
                        "Working time: %d ms", progress));
                if (fromUser) {
                    String command = "SLC_WT:" + progress;
                    btContext.sendCommand(command);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        qsEnabledSwitch = (Switch)findViewById(R.id.qsEnabledSwitch);
        qsEnabledSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean isChecked) {
                String command = "SQS:" + (isChecked ? "ON" : "OFF");
                btContext.sendCommand(command);
            }
        });

        qsMinRpmTextView = (TextView)findViewById(R.id.qsMinRpmTextView);
        qsMinRpmSeekBar = (SeekBar)findViewById(R.id.qsMinRpmSeekBar);
        qsMinRpmSeekBar.setMax(16);
        qsMinRpmSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                progress = progress * 250 + 2000;
                qsMinRpmTextView.setText(String.format(Locale.getDefault(),
                        "Minimum RPM: %d", progress));
                if (fromUser) {
                    String command = "SQS_MIN:" + progress;
                    btContext.sendCommand(command);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        qsMaxRpmTextView = (TextView)findViewById(R.id.qsMaxRpmTextView);
        qsMaxRpmSeekBar = (SeekBar)findViewById(R.id.qsMaxRpmSeekBar);
        qsMaxRpmSeekBar.setMax(40);
        qsMaxRpmSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                progress = progress * 250 + 6000;
                qsMaxRpmTextView.setText(String.format(Locale.getDefault(),
                        "Maximum RPM: %d", progress));
                if (fromUser) {
                    String command = "SQS_MAX:" + progress;
                    btContext.sendCommand(command);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        qsSensorTextView = (TextView)findViewById(R.id.qsSensorTextView);
        qsSensorProgressBar = (ProgressBar)findViewById(R.id.qsSensorProgressBar);

        qsSensitivityTextView = (TextView)findViewById(R.id.qsSensitivityTextView);
        qsSensitivitySeekBar = (SeekBar)findViewById(R.id.qsSensitivitySeekBar);
        qsSensitivitySeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                qsSensitivityTextView.setText(String.format(Locale.getDefault(),
                        "Sensitivity: %d", progress));
                if (fromUser) {
                    String command = "SQS_SENS:" + progress;
                    btContext.sendCommand(command);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        qsKillTimesButton = (Button)findViewById(R.id.qsKillTimesButton);
        qsKillTimesButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent i = new Intent(Settings.this, QuickShifterKillTimes.class);
                startActivity(i);
            }
        });

        rpmTextView = (TextView)findViewById(R.id.rpmTextView);
        rpmProgressBar = (ProgressBar)findViewById(R.id.rpmProgressBar);

        saveButton = (Button)findViewById(R.id.saveButton);
        saveButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String command = "SAVE";
                btContext.sendCommand(command);
                finish();
            }
        });
        backButton = (Button)findViewById(R.id.backButton);
        backButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                finish();
            }
        });

        final Handler updateHandler = new Handler();
        updateHandler.post(new Runnable() {
            @Override
            public void run() {
                btContext.write("GYPR");
                String[] ypr = btContext.read().split(",");
                gyroTextView.setText(String.format(Locale.getDefault(),
                        "X: %s°   Y: %s°   Z: %s°", ypr[0], ypr[1], ypr[2]));

                btContext.write("GQS_CSENS");
                int sensor = Integer.parseInt(btContext.read());
                if (sensor > 0) {
                    if (sensor > qsSensorMax) {
                        qsSensorMax = sensor;
                        qsSensitivitySeekBar.setMax(qsSensorMax);
                    }
                    qsSensorProgressBar.setProgress(sensor / qsSensorMax * 100);
                }
                qsSensorTextView.setText(String.format(Locale.getDefault(),
                        "Sensor reading: %d / %d", sensor, qsSensorMax));

                btContext.write("GRPM");
                int rpm = Integer.parseInt(btContext.read());
                rpmTextView.setText(String.format(Locale.getDefault(),
                        "Current RPM: %d / %d", rpm, maxRpm));
                if (rpm > maxRpm) {
                    maxRpm = rpm;
                    rpmProgressBar.setMax(maxRpm);
                }
                rpmProgressBar.setProgress(rpm);

                updateHandler.postDelayed(this, 250);
            }
        });

        initValues();
    }

    private void initValues() {
        // wheelie control
        btContext.write("GWC");
        wcEnabledSwitch.setChecked(btContext.read().equals("ON"));

        btContext.write("GWC_ANGLE");
        wcAngleSeekBar.setProgress(Integer.parseInt(btContext.read()) - 10);

        btContext.write("GWC_KT");
        wcKillTimeSeekBar.setProgress((Integer.parseInt(btContext.read()) - 50) / 10);

        // launch control
        btContext.write("GLC_RPM");
        lcRpmSeekBar.setProgress((Integer.parseInt(btContext.read()) - 2000) / 250);

        btContext.write("GLC_KT");
        lcKillTimeSeekBar.setProgress((Integer.parseInt(btContext.read()) - 50) / 10);

        btContext.write("GLC_WT");
        lcWorkingTimeSeekBar.setProgress((Integer.parseInt(btContext.read()) - 1000) / 100);

        // quickshifter
        btContext.write("GQS");
        qsEnabledSwitch.setChecked(btContext.read().equals("ON"));

        btContext.write("GQS_MIN");
        qsMinRpmSeekBar.setProgress((Integer.parseInt(btContext.read()) - 2000) / 250);

        btContext.write("GQS_MAX");
        qsMaxRpmSeekBar.setProgress((Integer.parseInt(btContext.read()) - 6000) / 250);

        btContext.write("GQS_SENS");
        qsSensitivitySeekBar.setProgress(Integer.parseInt(btContext.read()));
    }
}
