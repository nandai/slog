package jp.printf.slog.service;

import android.preference.PreferenceActivity;
import java.util.List;

public class Main extends PreferenceActivity
{
    public void onBuildHeaders(List<Header> target)
    {
        super.onBuildHeaders(target);
//      loadHeadersFromResource(R.xml.headers, target);
        getFragmentManager()
            .beginTransaction()
            .replace(android.R.id.content, new Settings())
            .commit();
    }
}
