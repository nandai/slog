//
//  Slog.swift
//
//  Created by SHINOHARA Yuki on 2016/02/09.
//  (C) 2016 printf.jp
//
import Foundation
import Starscream

/**
 * シーケンスログ用WebSocket
 */
private class SlogWebSocket: WebSocket, WebSocketDelegate
{
    private let fileName: String
    private let logLevel: Slog.LogLevel
    private let userName: String
    private let password: String
    private let dataPool = NSMutableArray()
    private var ready = false

    /**
     * 初期化
     *
     * - parameter  serviceAddr:    シーケンスログサービスへのURL（ex: ws://127.0.0.1:8080）
     * - parameter  fileName:       ログファイル名
     * - parameter  logLevel:       ログレベル
     * - parameter  userName:       ユーザー名
     * - parameter  password:       パスワード
     */
    init(_ serviceAddr: String, _ fileName: String, _ logLevel: Slog.LogLevel, _ userName: String, _ password: String)
    {
        self.fileName = fileName
        self.logLevel = logLevel
        self.userName = userName
        self.password = password

        super.init(url: NSURL(string: serviceAddr + "/outputLog")!)
        delegate = self;
    }

    /**
     * 接続時
     *
     * - parameter  socket: ソケット
     */
    private func websocketDidConnect(socket: WebSocket)
    {
        let data = NSMutableData()
        var strData: NSData!
        var len: UInt32

        // プロセスID送信
        var pid = CFSwapInt32BigToHost(1)
        var numData = NSData(bytes: &pid, length: 4)
        data.appendData(numData)

        // ユーザー名送信
        strData = self.userName.dataUsingEncoding(NSUTF8StringEncoding)
        len = CFSwapInt32BigToHost(UInt32(strData.length))
        numData = NSData(bytes: &len, length: 4)
        data.appendData(numData)
        data.appendData(strData)

        // パスワード送信
        strData = self.password.dataUsingEncoding(NSUTF8StringEncoding)
        len = CFSwapInt32BigToHost(UInt32(strData.length))
        numData = NSData(bytes: &len, length: 4)
        data.appendData(numData)
        data.appendData(strData)

        // シーケンスログファイル名送信
        strData = self.fileName.dataUsingEncoding(NSUTF8StringEncoding)
        len = CFSwapInt32BigToHost(UInt32(strData.length))
        numData = NSData(bytes: &len, length: 4)
        data.appendData(numData)
        data.appendData(strData)

        // ログレベル送信
        var logLevel = CFSwapInt32BigToHost(UInt32(bitPattern: self.logLevel.rawValue))
        numData = NSData(bytes: &logLevel, length: 4)
        data.appendData(numData)

        writeData(data)
        sendAllData()
        ready = true
    }

    /**
     * 切断時
     *
     * - parameter  socket: ソケット
     * - parameter  error:  エラー
     */
    private func websocketDidDisconnect(socket: WebSocket, error: NSError?)
    {
        if let message = error?.localizedDescription
        {
            print("Slog: " + message)
        }
    }

    /**
     * メッセージ受信時
     *
     * - parameter  socket: ソケット
     * - parameter  text:   テキスト
     */
    private func websocketDidReceiveMessage(socket: WebSocket, text: String)
    {
    }

    /**
     * データ受信時
     *
     * - parameter  socket: ソケット
     * - parameter  data:   データ
     */
    private func websocketDidReceiveData(socket: WebSocket, data: NSData)
    {
    }

    /**
     * 保留中の全データを送信
     */
    private func sendAllData()
    {
        for data in dataPool
        {
            writeData(data as! NSData)
        }
        
        dataPool.removeAllObjects()
    }
    
    /**
     * 送信
     *
     * - parameter  data:   データ
     */
    func sendData(data: NSData)
    {
        if  ready && isConnected
        {
            writeData(data)
        }
        else
        {
            dataPool.addObject(data)
        }
    }
}

/**
 * シーケンスログ
 */
class Slog
{
    private struct Static
    {
        static var ws: SlogWebSocket!
        static var no = 0
        static var logLevel = LogLevel.All
    }

    /**
     * ログレベル
     */
    internal enum LogLevel: Int32
    {
        case All = -1
        case Debug
        case Info
        case Warn
        case Error
        case None
    }

    /// ログレベル
    static var logLevel: LogLevel
    {
        get {return Static.logLevel}
        set {Static.logLevel = newValue}
    }

    /// シーケンスNo
    private var no: Int = 0

    /// スレッドNo
    private var tno: Int = -1

    /**
     * コンフィグ
     *
     * - parameter  serviceAddr:    シーケンスログサービスへのURL（ex: ws://127.0.0.1:8080）
     * - parameter  fileName:       ログファイル名
     * - parameter  logLevel:       ログレベル
     * - parameter  userName:       ユーザー名
     * - parameter  password:       パスワード
     */
    static func setConfig(serviceAddr: String, fileName: String, logLevel: LogLevel, userName: String, password: String)
    {
        Static.ws = SlogWebSocket(serviceAddr, fileName, logLevel, userName, password)
        Static.ws.connect()
    }

    /**
     * 初期化
     *
     * - parameter  className:  クラス名
     * - parameter  methdName:  メソッド名
     */
    init(_ className: String, _ methodName: String = __FUNCTION__)
    {
#if DEBUG
        stepIn(className, methodName)
#endif
    }

    /**
     * ステップイン
     *
     * - parameter  className:  クラス名
     * - parameter  methdName:  メソッド名
     */
    private func stepIn(className: String, _ methodName: String)
    {
        self.no = ++Static.no
        self.tno = getCurrentThreadNo()

        let data = NSMutableData()

        // シーケンス番号
        var no = CFSwapInt32BigToHost(UInt32(self.no))
        var numData = NSData(bytes: &no, length: 4)
        data.appendData(numData)

        // シーケンスログアイテム種別
        var type: Int8 = 0  // STEP_IN
        numData = NSData(bytes: &type, length: 1)
        data.appendData(numData)

        // スレッドNo
        no = CFSwapInt32BigToHost(UInt32(self.tno))
        numData = NSData(bytes: &no, length: 4)
        data.appendData(numData)

        // クラス名
        var id = 0
        numData = NSData(bytes: &id, length: 4)
        data.appendData(numData)

        var strData: NSData! = className.dataUsingEncoding(NSUTF8StringEncoding)
        var len = CFSwapInt16BigToHost(UInt16(strData.length))
        numData = NSData(bytes: &len, length: 2)
        data.appendData(numData)
        data.appendData(strData)

        // メソッド名
        numData = NSData(bytes: &id, length: 4)
        data.appendData(numData)

        strData = methodName.dataUsingEncoding(NSUTF8StringEncoding)
        len = CFSwapInt16BigToHost(UInt16(strData.length))
        numData = NSData(bytes: &len, length: 2)
        data.appendData(numData)
        data.appendData(strData)

        //
        let data2 = NSMutableData()
        len = CFSwapInt16BigToHost(UInt16(data.length + 2))
        numData = NSData(bytes: &len, length: 2)
        data2.appendData(numData)
        data2.appendData(data)

        Static.ws.sendData(data2)
    }

    deinit
    {
        if self.no != 0
        {
            stepOut()
        }
    }

    /**
     * ステップアウト
     *
     * スコープを外れたことを示すログを出力する。
     *
     * メソッドを抜ける際に明示的なstepOut呼び出しを忘れていても、整合性の合わないログにならないようにdeinitでstepOutを呼び出す。
     *
     * もっとも、let log = Slog(CLS_NAME)のように変数に代入していれば、メソッドを抜けるタイミングでdeinitが呼ばれるので、
     * 明示的にstepOutを呼びだす必要はない。
     *
     * ただし一度もlogを使用していなければ以下の警告が発生するので、それが気になるなら明示的にstepOutを呼び出すようにする（複数のreturnがあっても
     * それらすべての箇所でstepOutを呼びだす必要はない）。
     *
     *     Initialization of immutable value 'log' was never used; consider replacing with assignment to '_' or removing it
     *
     * 気にしているのは上記の警告を回避するために_ = Slog(CLS_NAME)としてしまわないかということであり、
     * deinitでstepOutを呼び出すというガード処理がなければ、冒頭で述べたとおり整合性の合わないログになってしまう。
     * また、_ = Slogとした場合には直後にdeinitされてしまい、メソッドを抜けたところでログを出力するという目的が果たせないので推奨しない。
     */
    func stepOut()
    {
#if DEBUG
        var no = UInt32(self.no)
        self.no = 0

        let data = NSMutableData()

        // シーケンス番号
        no = CFSwapInt32BigToHost(UInt32(no))
        var numData = NSData(bytes: &no, length: 4)
        data.appendData(numData)

        // シーケンスログアイテム種別
        var type: Int8 = 1  // STEP_OUT
        numData = NSData(bytes: &type, length: 1)
        data.appendData(numData)

        // スレッドNo
        no = CFSwapInt32BigToHost(UInt32(self.tno))
        numData = NSData(bytes: &no, length: 4)
        data.appendData(numData)

        //
        let data2 = NSMutableData()
        var len = CFSwapInt16BigToHost(UInt16(data.length + 2))
        numData = NSData(bytes: &len, length: 2)
        data2.appendData(numData)
        data2.appendData(data)

        Static.ws.sendData(data2)
#endif
    }

    /**
     * デバッグメッセージを出力する
     *
     * - parameter  aMessage:   メッセージ
     */
    func d(@autoclosure aMessage: () -> String)
    {
#if DEBUG
        if Static.logLevel.rawValue <= LogLevel.Debug.rawValue
        {
            message(LogLevel.Debug, aMessage())
        }
#endif
    }

    /**
     * 情報メッセージを出力する
     *
     * - parameter  aMessage:   メッセージ
     */
    func i(@autoclosure aMessage: () -> String)
    {
#if DEBUG
        if Static.logLevel.rawValue <= LogLevel.Info.rawValue
        {
            message(LogLevel.Info, aMessage())
        }
#endif
    }

    /**
     * 警告メッセージを出力する
     *
     * - parameter  aMessage:   メッセージ
     */
    func w(@autoclosure aMessage: () -> String)
    {
#if DEBUG
        if Static.logLevel.rawValue <= LogLevel.Warn.rawValue
        {
            message(LogLevel.Warn, aMessage())
        }
#endif
    }

    /**
     * エラーメッセージを出力する
     *
     * - parameter  aMessage:   メッセージ
     */
    func e(@autoclosure aMessage: () -> String)
    {
#if DEBUG
        if Static.logLevel.rawValue <= LogLevel.Error.rawValue
        {
            message(LogLevel.Error, aMessage())
        }
#endif
    }

    /**
     * メッセージを出力する
     *
     * - parameter  level:      ログレベル
     * - parameter  message:    メッセージ
     */
    private func message(level: LogLevel, _ message: String)
    {
        let data = NSMutableData()
        var no = UInt32(self.no)

        // シーケンス番号
        no = CFSwapInt32BigToHost(UInt32(no))
        var numData = NSData(bytes: &no, length: 4)
        data.appendData(numData)
        
        // シーケンスログアイテム種別
        var type: Int8 = 2  // MESSAGE
        numData = NSData(bytes: &type, length: 1)
        data.appendData(numData)
        
        // スレッドNo
        no = CFSwapInt32BigToHost(UInt32(self.tno))
        numData = NSData(bytes: &no, length: 4)
        data.appendData(numData)

        // メッセージ
        var lvl = level.rawValue
        numData = NSData(bytes: &lvl, length: 1)
        data.appendData(numData)

        var id = 0
        numData = NSData(bytes: &id, length: 4)
        data.appendData(numData)

        let strData: NSData! = message.dataUsingEncoding(NSUTF8StringEncoding)
        var len = CFSwapInt16BigToHost(UInt16(strData.length))
        numData = NSData(bytes: &len, length: 2)
        data.appendData(numData)
        data.appendData(strData)

        //
        let data2 = NSMutableData()
        len = CFSwapInt16BigToHost(UInt16(data.length + 2))
        numData = NSData(bytes: &len, length: 2)
        data2.appendData(numData)
        data2.appendData(data)

        Static.ws.sendData(data2)
    }

    /**
     * 現在のスレッドNoを取得する
     *
     * - returns:   スレッドNo
     */
    private func getCurrentThreadNo() -> Int
    {
        let currentThread = NSThread.currentThread()
        let range = currentThread.description.rangeOfString("{number = ")
        var tno: Int = 0

        if range != nil
        {
            let value = currentThread.description.substringFromIndex((range?.endIndex)!)
            tno = Int((value as NSString).intValue)
        }

        return tno
    }
}
