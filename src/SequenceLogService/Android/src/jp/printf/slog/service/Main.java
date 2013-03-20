/*
 * Copyright (C) 2011-2013 printf.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
