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
import android.widget.Toast;

import java.util.Locale;


public class Settings extends AppCompatActivity {
    BluetoothContext btContext;
    Switch wcEnabledSwitch, qsEnabledSwitch;
    TextView gyroTextView, killTimeTextView, wcAngleTextView, wcKillTimeTextView, lcRpmTextView,
            lcKillTimeTextView, lcWorkingTimeTextView, rpmTextView, qsMinRpmTextView,
            qsMaxRpmTextView, qsSensorTextView, qsSensitivityTextView;
    SeekBar killTimeSeekBar, wcAngleSeekBar, wcKillTimeSeekBar, lcRpmSeekBar, lcKillTimeSeekBar,
            lcWorkingTimeSeekBar, qsMinRpmSeekBar, qsMaxRpmSeekBar, qsSensitivitySeekBar;
    ProgressBar qsSensorProgressBar, rpmProgressBar;
    Button gyroButton, killTimeButton, qsKillTimesButton, saveButton, backButton;

    private int maxRpm = 0;
    private int qsSensorMax = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);

        btContext = MainScreen.btContext;
        if (!btContext.isReady()) {
            Toast.makeText(getApplicationContext(), "Connection lost", Toast.LENGTH_SHORT).show();
            finish();
            return;
        }

        gyroTextView = (TextView)findViewById(R.id.gyroTextView);
        gyroButton = (Button)findViewById(R.id.gyroButton);
        gyroButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String command = "CAL_GYRO";
                btContext.sendCommand(command);
            }
        });

        killTimeTextView = (TextView)findViewById(R.id.killTimeTextView);
        killTimeSeekBar = (SeekBar) findViewById(R.id.killTimeSeekBar);
        killTimeSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                progress *= 5;
                killTimeTextView.setText(String.format(Locale.getDefault(),
                        "Kill time: %d ms", progress));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });
        killTimeButton = (Button)findViewById(R.id.killTimeButton);
        killTimeButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                int kill_time = killTimeSeekBar.getProgress() * 5;
                String command = "SKS:" + kill_time;
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
                String[] ypr = btContext.sendQuery("GYPR").split(",");
                gyroTextView.setText(String.format(Locale.getDefault(),
                        "X: %s°   Y: %s°   Z: %s°", ypr[0], ypr[1], ypr[2]));

                int sensor = Integer.parseInt(btContext.sendQuery("GQS_CSENS"));
                if (sensor > 0) {
                    if (sensor > qsSensorMax) {
                        qsSensorMax = sensor;
                        qsSensitivitySeekBar.setMax(qsSensorMax);
                    }
                    qsSensorProgressBar.setProgress(sensor / qsSensorMax * 100);
                }
                qsSensorTextView.setText(String.format(Locale.getDefault(),
                        "Sensor reading: %d / %d", sensor, qsSensorMax));

                int rpm = Integer.parseInt(btContext.sendQuery("GRPM"));
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
        wcEnabledSwitch.setChecked(btContext.sendQuery("GWC").equals("ON"));
        wcAngleSeekBar.setProgress(Integer.parseInt(btContext.sendQuery("GWC_ANGLE")) - 10);
        wcKillTimeSeekBar.setProgress((Integer.parseInt(btContext.sendQuery("GWC_KT")) - 50) / 10);

        // launch control
        lcRpmSeekBar.setProgress((Integer.parseInt(btContext.sendQuery("GLC_RPM")) - 2000) / 250);
        lcKillTimeSeekBar.setProgress((Integer.parseInt(btContext.sendQuery("GLC_KT")) - 50) / 10);
        lcWorkingTimeSeekBar.setProgress((Integer.parseInt(btContext.sendQuery("GLC_WT")) - 1000) / 100);

        // quick shifter
        qsEnabledSwitch.setChecked(btContext.sendQuery("GQS").equals("ON"));
        qsMinRpmSeekBar.setProgress((Integer.parseInt(btContext.sendQuery("GQS_MIN")) - 2000) / 250);
        qsMaxRpmSeekBar.setProgress((Integer.parseInt(btContext.sendQuery("GQS_MAX")) - 6000) / 250);
        qsSensitivitySeekBar.setProgress(Integer.parseInt(btContext.sendQuery("GQS_SENS")));
    }
}
