package example.com.networkinfo;

import org.json.*;
import java.util.List;
import android.content.Context;
import android.telephony.TelephonyManager;
import android.telephony.CellInfo;
import android.telephony.CellInfoGsm;
import android.telephony.CellIdentityGsm;
import android.telephony.CellSignalStrengthGsm;

public class NetworkInfo extends org.qtproject.qt5.android.bindings.QtActivity
{
    private static NetworkInfo m_instance;
    private static TelephonyManager m_mgr;

    public NetworkInfo()
    {
        m_instance = this;
    }

    public static String getInfo()
    {
        if(m_mgr == null) {
            m_mgr = (TelephonyManager)m_instance.getSystemService(Context.TELEPHONY_SERVICE);
        }

        String errorMessage = "";
        JSONObject response = new JSONObject();
        JSONArray cellList = new JSONArray();
        List<CellInfo> data = m_mgr.getAllCellInfo();

        for(CellInfo i : data) {
            if(i instanceof CellInfoGsm) {
                try {
                    JSONObject cell = new JSONObject();
                    CellIdentityGsm id = ((CellInfoGsm)i).getCellIdentity();
                    CellSignalStrengthGsm sig = ((CellInfoGsm)i).getCellSignalStrength();
                    cell.put("mcc", id.getMcc()); // Mobile country code
                    cell.put("mnc", id.getMnc()); // Mobile network code
                    cell.put("cid", id.getCid()); // Cell identity
                    cell.put("lac", id.getLac()); // Location area code
                    cell.put("ss", sig.getDbm()); // Signal strength in dbm
                    cellList.put(cell);
                } catch(Exception e) {
                    errorMessage = e.getMessage();
                    break;
                }
            } else {
                errorMessage = "Set your phone to 2G/GSM only mode";
                break;
            }
        }

        try {
            if(errorMessage.length() != 0) {
                response.put("error", errorMessage);
            } else {
                response.put("cells", cellList);
            }
        } catch(Exception e) {
            System.out.println(e.getMessage());
            return "";
        }

        return response.toString();
    }
}
