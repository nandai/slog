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

import android.content.Context;
import android.content.Intent;
import android.content.res.TypedArray;
import android.net.Uri;
import android.preference.Preference;
import android.util.AttributeSet;

public class LinkPreference extends Preference
{
    private String  mUrl;

    /**
     * コンストラクタ
     */
    public LinkPreference(Context context)
    {
        this(context, null);
    }

    /**
     * コンストラクタ
     */
    public LinkPreference(Context context, AttributeSet attrs)
    {
        super(context, attrs);

        TypedArray ta = context.obtainStyledAttributes(attrs, R.styleable.LinkPreference);
        mUrl = ta.getString(R.styleable.LinkPreference_url);

        ta.recycle();
    }

    @Override
    protected void onClick()
    {
        Uri uri = Uri.parse(mUrl);
        Intent intent = new Intent(Intent.ACTION_VIEW, uri);

        getContext().startActivity(intent);
    }

    /**
     * URL設定
     * @param   url URL
     */
    public void setUrl(String url)
    {
        mUrl = url;
    }
}
