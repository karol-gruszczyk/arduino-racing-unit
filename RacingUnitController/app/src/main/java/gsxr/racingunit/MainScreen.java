package gsxr.racingunit;

import android.content.Intent;
import android.graphics.Color;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.RelativeLayout;
import android.widget.Switch;
import android.widget.TextView;

import java.util.Locale;


public class MainScreen extends AppCompatActivity {
    static BluetoothContext btContext;
    Button settingsButton, lightsButton;
    Switch lcEnabledSwitch;
    RelativeLayout background;
    TextView wcAngleTextView, tiltAngleTextView;
    boolean lightsEnabled = true;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        View v = getLayoutInflater().inflate(R.layout.activity_main_screen, null);// or any View (incase generated programmatically )
        v.setKeepScreenOn(true);
        setContentView(v);

        btContext = new BluetoothContext(getApplicationContext(), this);
        btContext.execute();

        background = (RelativeLayout)findViewById(R.id.background);

        wcAngleTextView = (TextView)findViewById(R.id.wcAngleTextView);
        tiltAngleTextView = (TextView)findViewById(R.id.tiltAngleTextView);

        settingsButton = (Button)findViewById(R.id.settingsButton);
        settingsButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent i = new Intent(MainScreen.this, Settings.class);
                startActivity(i);
            }
        });

        lightsButton = (Button)findViewById(R.id.lightsButton);
        lightsButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                lightsEnabled = !lightsEnabled;
                background.setBackgroundColor(lightsEnabled ? Color.WHITE : Color.BLACK);
                for (int i = 0; i < background.getChildCount(); i++) {
                    View v = background.getChildAt(i);
                    if (v instanceof TextView) {
                        TextView child = (TextView)v.findViewById(v.getId());
                        child.setTextColor(lightsEnabled ? Color.BLACK : Color.LTGRAY);
                    }
                }
            }
        });

        lcEnabledSwitch = (Switch)findViewById(R.id.lcSwitch);
        lcEnabledSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean isChecked) {
                String command = "SLC:" + (isChecked ? "ON" : "OFF");
                btContext.sendCommand(command);
            }
        });

        final Handler updateHandler = new Handler();
        updateHandler.post(new Runnable() {
            @Override
            public void run() {
                if (btContext.isReady()) {
                    btContext.write("GLC");
                    lcEnabledSwitch.setChecked(btContext.read().equals("ON"));

                    btContext.write("GYPR");
                    String[] ypr = btContext.read().split(",");
                    wcAngleTextView.setText(String.format(Locale.getDefault(), "%s°", ypr[2]));
                    tiltAngleTextView.setText(String.format(Locale.getDefault(), "%s°", ypr[1]));
                }

                updateHandler.postDelayed(this, 250);
            }
        });

    }
}
