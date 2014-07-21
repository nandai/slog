<?php

/*
 * Copyright (C) 2013-2014 printf.jp
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
$DEBUG = 0;

/**
 * モデルクラス
 */
class Model
{
    private static  $CLS_NAME = 'Model';

    private $pattern;           // 正解文字列生成パターン
    private $correctSize = 1;   // 正解文字列の桁
    private $correct;           // 正解文字列
    private $answer;            // 返答

    /**
     * コンストラクタ
     */
    function __construct()
    {
        $TAG = slogStepIn(self::$CLS_NAME, '__construct');

        $this->pattern = '0123456789';  // アルファベットもまぜたかったら
                                        // '0123456789ABCDEF' とする。

        $this->correct = array();
        $this->answer =  array();

        slogStepOut($TAG);
    }

    /**
     * 正解文字列の桁を取得
     */
    function getCorrectSize()
    {
        $TAG = slogStepIn(self::$CLS_NAME, 'getCorrectSize');
        slogStepOut($TAG);
        return $this->correctSize;
    }

    /**
     * 正解文字列の桁を設定
     */
    function setCorrectSize($correctSize)
    {
        $TAG = slogStepIn(self::$CLS_NAME, 'setCorrectSize');
        slogD($TAG, '正解文字列の桁:' . $correctSize);

        $this->correctSize = $correctSize;
        slogStepOut($TAG);
    }

    /**
     * 正解文字列生成
     */
    function createCorrect()
    {
        $TAG = slogStepIn(self::$CLS_NAME, 'createCorrect');

        global $DEBUG;
        $len = strlen($this->pattern);

        for ($i = 0; $i < $this->correctSize; $i++)
        {
            $value = mt_rand(0, $len - 1);
            $this->correct[$i] = $this->pattern[$value];
        }

        if ($DEBUG === 1)
        {
            print('デバッグモード ON' . PHP_EOL);
            print('答えは... ' . join('', $this->correct) . PHP_EOL);
            print(PHP_EOL);
        }

        slogD($TAG, '生成した正解文字列:' . join('', $this->correct));
        slogStepOut($TAG);
    }

    /**
     * 正解文字列取得
     */
    function getCorrect()
    {
        $TAG = slogStepIn(self::$CLS_NAME, 'getCorrect');
        slogStepOut($TAG);
        return $this->correct;
    }

    /**
     * 返答設定
     * 成否判定は行わない
     */
    function setAnswer($answer)
    {
        $TAG = slogStepIn(self::$CLS_NAME, 'setAnswer');
        slogD($TAG, '入力:' . $answer);

        $len = strlen($answer);

        if ($len != $this->correctSize)
        {
            // 桁数不正
            slogStepOut($TAG);
            return false;
        }

        for ($i = 0; $i < $this->correctSize; $i++)
        {
            if (strpos($this->pattern, $answer[$i]) === FALSE)
            {
                // 文字不正
                slogStepOut($TAG);
                return false;
            }
        }

        for ($i = 0; $i < $this->correctSize; $i++)
            $this->answer[$i] = $answer[$i];

        slogStepOut($TAG);
        return true;
    }

    /**
     * hit数取得
     */
    private function getHitCount(&$result)
    {
        $TAG = slogStepIn(self::$CLS_NAME, 'getHitCount');
        $hit = 0;

        foreach ($this->answer as $key => $value)
        {
            if ($this->correct[$key] === $value)
            {
                $result[$key] = $value;
                $hit++;
            }
            else
            {
                $result[$key] = '*';
            }
        }

        slogD($TAG, 'hit数:' . $hit);
        slogStepOut($TAG);
        return $hit;
    }

    /**
     * chip数取得
     */
    private function getChipCount(&$result)
    {
        $TAG = slogStepIn(self::$CLS_NAME, 'getChipCount');
        $chip = 0;

        foreach ($this->answer as $key => $value)
        {
            foreach ($this->correct as $coKey => $coValue)
            {
                if ($result[$coKey] !== '*')
                    continue;

                if ($coValue === $value)
                {
                    $result[$coKey] = '?';
                    $chip++;
                    break;
                }
            }
        }

        slogD($TAG, 'chip数:' . $chip);
        slogStepOut($TAG);
        return $chip;
    }

    /**
     * 結果取得
     */
    function getResult(&$hit, &$chip)
    {
        $TAG = slogStepIn(self::$CLS_NAME, 'getResult');
        $result = array();

        $hit =  $this->getHitCount( $result);
        $chip = $this->getChipCount($result);

        slogD($TAG, join('', $result));
        slogStepOut($TAG);
    }

    /**
     * ヒント取得
     */
    function getHint()
    {
        $TAG = slogStepIn(self::$CLS_NAME, 'getHint');
        $result = array();

        $this->getHitCount( $result);
        $this->getChipCount($result);

        slogStepOut($TAG);
        return join('', $result);
    }
}

/**
 * ビュークラス
 */
class View
{
    private static  $CLS_NAME = 'View';

    private $model = null;      // モデル

    /**
     * モデル設定
     */
    function setModel(&$model)
    {
        $TAG = slogStepIn(self::$CLS_NAME, 'setModel');
        $this->model = $model;
        slogStepOut($TAG);
    }

    /**
     * 正解した場合の表示
     */
    function showCorrect()
    {
        $TAG = slogStepIn(self::$CLS_NAME, 'showCorrect');
        print('正解！' . PHP_EOL);
        slogStepOut($TAG);
    }

    /**
     * 失敗した場合の表示
     */
    function showFailed()
    {
        $TAG = slogStepIn(self::$CLS_NAME, 'showFailed');
        print('正解は ');

        $correct = $this->model->getCorrect();

        for ($i = 0; $i < count($correct); $i++)
            print($correct[$i]);

        print(' でした。' . PHP_EOL);
        slogStepOut($TAG);
    }

    /**
     * 結果表示
     */
    function showResult($hit, $chip)
    {
        $TAG = slogStepIn(self::$CLS_NAME, 'showResult');

        print('HIT数は ' . $hit . ', CHIP数は ' . $chip . ' です。' . PHP_EOL);
        print(PHP_EOL);

        slogStepOut($TAG);
    }

    /**
     * 入力不正表示
     */
    function showWrong()
    {
        $TAG = slogStepIn(self::$CLS_NAME, 'showWrong');
        print('入力が正しくありません。もう一度入力してください。' . PHP_EOL);
        slogStepOut($TAG);
    }

    /**
     * ヒント表示
     */
    function showHint()
    {
        $TAG = slogStepIn(self::$CLS_NAME, 'showHint');
        $hint = $this->model->getHint();

        print($hint . PHP_EOL);
        print(PHP_EOL);

        slogStepOut($TAG);
    }
}

/**
 * コントローラクラス
 */
class Controller
{
    private static  $CLS_NAME = 'Controller';

    private $model;             // モデル
    private $view;              // ビュー

    private $tryMaxCount = 1;   // 返答試行最大数
    private $tryCount;          // 返答試行回数

    /**
     * コンストラクタ
     */
    function __construct()
    {
        $TAG = slogStepIn(self::$CLS_NAME, '__construct');

        $this->model = new Model;
        $this->view = new View;
        $this->view->setModel($this->model);

        slogStepOut($TAG);
    }

    /**
     * 返答試行最大数設定
     */
    function setTryMaxCount($tryMaxCount)
    {
        $TAG = slogStepIn(self::$CLS_NAME, 'setTryMaxCount');
        slogD($TAG, '返答試行最大数:' . $tryMaxCount);

        $this->tryMaxCount = $tryMaxCount;
        slogStepOut($TAG);
    }

    /**
     * 正解文字列の桁を設定
     */
    function setCorrectSize($correctSize)
    {
        $TAG = slogStepIn(self::$CLS_NAME, 'setCorrectSize');
        $this->model->setCorrectSize($correctSize);
        slogStepOut($TAG);
    }

    /**
     * 返答取得
     */
    private function getAnswer()
    {
        $TAG = slogStepIn(self::$CLS_NAME, 'getAnswer');

        // 返答待ちメッセージ表示
        $message = $this->tryCount . ' 回目 > ';
        print(      $message);
        slogD($TAG, $message);

        // 標準入力より返答取得
        $answer = fgets(STDIN, 1024);
        $answer = str_replace(PHP_EOL, '', $answer);

        slogStepOut($TAG);
        return $answer;
    }

    /**
     * 開始
     */
    function start()
    {
        $TAG = slogStepIn(self::$CLS_NAME, 'start');
        print('"HELP!"と入力するとヒントを表示します。' . PHP_EOL . PHP_EOL);

        $this->model->createCorrect();
        $this->tryCount = 1;
        $beforeAnswer = '';

        while (true)
        {
            if ($this->tryMaxCount < $this->tryCount)
            {
                // 返答試行最大数を超えたので終了
                $this->view->showFailed();
                break;
            }

            // 入力チェック
            $answer = $this->getAnswer();

            if (strcmp($answer, 'HELP!') === 0)
            {
                $this->model->setAnswer($beforeAnswer);
                $this->view->showHint();
                continue;
            }

            if ($this->model->setAnswer($answer) == false)
            {
                // 入力不正。試行回数は増やさず再入力を促す
                $this->view->showWrong();
                continue;
            }

            // 結果判定
            $this->model->getResult($hit, $chip);

            if ($hit === $this->model->getCorrectSize())
            {
                // 完全一致
                $this->view->showCorrect();
                break;
            }
            else
            {
                // 不一致
                $this->view->showResult($hit, $chip);
            }

            $beforeAnswer = $answer;
            $this->tryCount++;
        }

        slogStepOut($TAG);
    }
}

/**
 * アプリケーションクラス
 */
class Application
{
    private static  $CLS_NAME = 'Application';

    private static  $TRY_MAX_COUNT = 10;    // 返答試行最大数
    static private  $CORRECT_SIZE =   4;    // 正解文字列の桁

    private         $controller;            // コントローラ

    /**
     * 初期化
     */
    private function init()
    {
        $TAG = slogStepIn(self::$CLS_NAME, 'init');

        $this->controller = new Controller;
        $this->controller->setTryMaxCount(self::$TRY_MAX_COUNT);
        $this->controller->setCorrectSize(self::$CORRECT_SIZE);

        slogStepOut($TAG);
    }

    /**
     * 開始
     */
    function start()
    {
        slogLoadConfig('HitChips.log.config');
        $TAG = slogStepIn(self::$CLS_NAME, 'start');

        slogAssert($TAG, 'slogAssert(true)',  true);
        slogAssert($TAG, 'slogAssert(false)', false);

        $this->init();
        $this->controller->start();

        slogStepOut($TAG);
    }
}

/**
 * 処理開始
 */
{
    $app = new Application;
    $app->start();
}

?>
