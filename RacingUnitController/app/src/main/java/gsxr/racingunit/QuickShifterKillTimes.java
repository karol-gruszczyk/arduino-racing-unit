package gsxr.racingunit;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Locale;


public class QuickShifterKillTimes extends AppCompatActivity {
    BluetoothContext btContext;
    ArrayList<SeekBar> seekBarArrayList;
    ArrayList<TextView> textViewArrayList;
    Button backButton;
    Button setButton;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_quick_shifter_kill_times);

        btContext = MainScreen.btContext;

        TextView rpm2500TextView = (TextView)findViewById(R.id.rpm2500TextView);
        TextView rpm4000TextView = (TextView)findViewById(R.id.rpm4000TextView);
        TextView rpm5500TextView = (TextView)findViewById(R.id.rpm5500TextView);
        TextView rpm7000TextView = (TextView)findViewById(R.id.rpm7000TextView);
        TextView rpm8500TextView = (TextView)findViewById(R.id.rpm8500TextView);
        TextView rpm10000TextView = (TextView)findViewById(R.id.rpm10000TextView);
        TextView rpm11500TextView = (TextView)findViewById(R.id.rpm11500TextView);
        TextView rpm13000TextView = (TextView)findViewById(R.id.rpm13000TextView);

        SeekBar rpm2500SeekBar = (SeekBar)findViewById(R.id.rpm2500SeekBar);
        SeekBar rpm4000SeekBar = (SeekBar)findViewById(R.id.rpm4000SeekBar);
        SeekBar rpm5500SeekBar = (SeekBar)findViewById(R.id.rpm5500SeekBar);
        SeekBar rpm7000SeekBar = (SeekBar)findViewById(R.id.rpm7000SeekBar);
        SeekBar rpm8500SeekBar = (SeekBar)findViewById(R.id.rpm8500SeekBar);
        SeekBar rpm10000SeekBar = (SeekBar)findViewById(R.id.rpm10000SeekBar);
        SeekBar rpm11500SeekBar = (SeekBar)findViewById(R.id.rpm11500SeekBar);
        SeekBar rpm13000SeekBar = (SeekBar)findViewById(R.id.rpm13000SeekBar);

        seekBarArrayList = new ArrayList<>(Arrays.asList(rpm2500SeekBar, rpm4000SeekBar,
                rpm5500SeekBar, rpm7000SeekBar, rpm8500SeekBar,
                rpm10000SeekBar, rpm11500SeekBar, rpm13000SeekBar));
        textViewArrayList = new ArrayList<>(Arrays.asList(rpm2500TextView, rpm4000TextView,
                rpm5500TextView, rpm7000TextView, rpm8500TextView,
                rpm10000TextView, rpm11500TextView, rpm13000TextView));

        backButton = (Button)findViewById(R.id.backButton);
        setButton = (Button)findViewById(R.id.setButton);

        backButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                finish();
            }
        });

        setButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String command = "SQS_KT:";
                ArrayList<String> killTimes = new ArrayList<>();
                for (SeekBar seekBar : seekBarArrayList)
                    killTimes.add(String.valueOf(progressToHuman(seekBar.getProgress())));
                command += TextUtils.join(",", killTimes);
                btContext.sendCommand(command);
                finish();
            }
        });

        int i = 0;
        btContext.write("GQS_KT");
        String[] killTimes = btContext.read().split(",");
        for (SeekBar seekBar : seekBarArrayList) {
            seekBar.setMax(20);
            seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                @Override
                public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                    textViewArrayList.get(seekBarArrayList.indexOf(seekBar)).setText(
                            String.format(Locale.getDefault(), "%d ms",
                                    progressToHuman(progress))
                    );
                }

                @Override
                public void onStartTrackingTouch(SeekBar seekBar) {
                }

                @Override
                public void onStopTrackingTouch(SeekBar seekBar) {
                }
            });
            seekBar.setProgress(humanToProgress(Integer.parseInt(killTimes[i++])));
        }
    }

    private int progressToHuman(int progress) {
        return progress * 5 + 40;
    }

    private int humanToProgress(int human) {
        return (human - 40) / 5;
    }
}
