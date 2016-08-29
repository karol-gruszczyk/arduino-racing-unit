package gsxr.racingunit;


import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.os.AsyncTask;
import android.support.v7.app.AppCompatActivity;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.UUID;


public class BluetoothContext extends AsyncTask<Void, Void, Void> { // UI thread
    private boolean ConnectSuccess = true;
    private ProgressDialog progress;
    BluetoothAdapter myBluetooth = null;
    BluetoothSocket btSocket = null;
    private boolean isBtConnected = false;
    static final UUID myUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    Context applicationContext;
    AppCompatActivity parent;

    BluetoothContext(Context applicationContext, AppCompatActivity parent) {
        super();
        this.applicationContext = applicationContext;
        this.parent = parent;
    }

    @Override
    protected void onPreExecute() {
        progress = ProgressDialog.show(parent, "Connecting...", "Please wait...");  //show a progress dialog
    }

    @Override
    protected Void doInBackground(Void... devices) {  // while the progress dialog is shown, the connection is done in background
        try {
            if (btSocket == null || !isBtConnected) {
                String address = parent.getIntent().getStringExtra(DeviceList.EXTRA_ADDRESS);
                myBluetooth = BluetoothAdapter.getDefaultAdapter();  // get the mobile bluetooth device
                BluetoothDevice dispositivo = myBluetooth.getRemoteDevice(address);  // connects to the device's address and checks if it's available
                btSocket = dispositivo.createInsecureRfcommSocketToServiceRecord(myUUID);  // create a RFCOMM (SPP) connection
                BluetoothAdapter.getDefaultAdapter().cancelDiscovery();
                btSocket.connect();  // start connection
            }
        }
        catch (IOException e) {
            e.printStackTrace();
            ConnectSuccess = false;  // if the try failed, you can check the exception here
            try {
                btSocket.close();
            } catch (IOException ee) {
                ee.printStackTrace();
            }
        }
        return null;
    }
    @Override
    protected void onPostExecute(Void result) {  // after the doInBackground, it checks if everything went fine
        super.onPostExecute(result);

        if (!ConnectSuccess) {
            Toast.makeText(applicationContext, "Connection Failed. Is it a SPP Bluetooth? Try again.", Toast.LENGTH_LONG).show();
            parent.finish();
        } else {
            Toast.makeText(applicationContext, "Connected.", Toast.LENGTH_LONG).show();
            isBtConnected = true;
        }
        progress.dismiss();
    }

    public boolean isReady() {
        return btSocket != null && isBtConnected;
    }

    protected void close() {
        if (isReady()) {
            try {
                btSocket.getOutputStream().close();
            } catch (IOException e) {
                Toast.makeText(applicationContext, "Error", Toast.LENGTH_LONG).show();
            }
        }
    }

    private void write(String command) {
        if (isReady()) {
            try {
                btSocket.getOutputStream().flush();
                btSocket.getOutputStream().write((command + '\n').getBytes());
            } catch (IOException e) {
                Toast.makeText(applicationContext, "Socket write IOException", Toast.LENGTH_LONG).show();
            }
        }
    }

    private String read() {
        if (isReady()) {
            try {
                BufferedReader reader = new BufferedReader(
                        new InputStreamReader(btSocket.getInputStream()));
                return reader.readLine();
            } catch(IOException e) {
                Toast.makeText(applicationContext, "Socket read IOException", Toast.LENGTH_LONG).show();
            }
        }
        return "Socket is null";
    }

    protected void sendCommand(String command) {
        write(command);
        String response = read();
        if (!response.equals("OK"))
            Toast.makeText(applicationContext, response, Toast.LENGTH_LONG).show();
    }

    protected String sendQuery(String query) {
        write(query);
        return read();
    }
}
