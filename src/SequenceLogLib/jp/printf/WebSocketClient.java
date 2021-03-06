/*
 * Copyright (C) 2013 printf.jp
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
package jp.printf;

import java.nio.ByteBuffer;

public class WebSocketClient
{
    WebSocketListener   mListener = null;
    long                mNativeObj;

    public void setListener(WebSocketListener listener)
    {
        mListener = listener;
    }

    public native void open(String url);
    public native void close();
    public native void send(String str);
    public native void send(ByteBuffer buffer);

    private void onOpen()
    {
        if (mListener != null)
            mListener.onOpen();
    }

    private void onError(String message)
    {
        if (mListener != null)
            mListener.onError(message);
    }

    private void onMessage(ByteBuffer buffer)
    {
        if (mListener != null)
            mListener.onMessage(buffer);
    }

    private void onClose()
    {
        if (mListener != null)
            mListener.onClose();
    }
}
