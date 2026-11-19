# CTA Spectator

Arma Reforger向けの観戦Modです。プレイヤー一覧から観戦先へカメラを移動でき、リスポーン回数の制限にも対応します。

## 主な機能

- 接続中のプレイヤー一覧から観戦先へカメラを移動
- 観戦カメラ付近のローカルボイスを受信
- 観戦終了時に通常の音声受信へ復帰
- プレイヤーごとの残機および勢力ごとのチケットによるリスポーン制限

## ミッションへの組み込み

1. ミッションのプロジェクトに`CTA_Spectator`を依存関係として追加します。
2. サーバーと全クライアントで、同じバージョンのModを読み込みます。
3. 使用するGameModeに`CTA_CallQueue`、`CTA_PlayerLifeManagerComponent`、`CTA_FactionTicketManagerComponent`、`CTA_RespawnSystemComponent`があることを確認します。Mod内の`Prefabs/MP/Modes/GameMode_Base.et`に追加されています。
4. 手動で観戦へ入る場所には、`ActionsManagerComponent`のアクションとして`CTA_EnableSpectatorAction`を設定します。アクションの表示名とコンテキストも設定してください。

配置例は[`worlds/Test.ent`](worlds/Test.ent)と、その`Test_Layers`フォルダーを参照してください。  
テストワールドをそのまま本番ミッションとして利用することは想定していません。

## リスポーン制限

初期設定は`Unlimited`（無制限）です。リスポーン方式、初期ライフ数、初期チケット数はMission Headerの`m_eRespawnType`、`m_iInitialLives`、`m_iInitialFactionTickets`で設定できます。

| 方式 | 管理者チャットコマンド | 内容 |
|---|---|---|
| 残機 | `#respawnType life` | プレイヤーごとの残機で制限 |
| 勢力チケット | `#respawnType ticket` | 所属勢力ごとのチケット数で制限 |
| 無制限 | `#respawnType unlimited` | ライフ・チケットによる観戦移行を無効化 |

Mission Headerでリスポーン方式、初期ライフ数、初期チケット数を指定できます。Scenario Frameworkで`respawnType`変数を設定した場合は、Mission Headerの方式よりScenario Frameworkの指定が優先されます。以前の`individual`指定も互換性のため利用できます。Ticket方式では、勢力チケットが残っている間はチケットのみを消費します。勢力チケットが0になった後、次の死亡からプレイヤーライフを消費し、ライフが0で再スポーンすると観戦へ移行します。Life方式ではライフのみ、Unlimited方式ではどちらも消費しません。

管理者は`#initialLife 数値`と`#initialTicket 数値`で、それぞれの初期値を変更できます。変更すると、接続中プレイヤーの残ライフまたは各勢力の残チケットも新しい値に更新されます。Ticket方式のフォールバックライフは`initialLife`設定にかかわらず1に固定されます。

リスポーン方式と残数はサーバー側で管理し、更新状態をクライアントへ配信します。プレイヤーライフと勢力チケットのManagerは残数のみを管理し、死亡イベントの処理は`CTA_RespawnSystemComponent`が担います。観戦者UIの一覧は接続中のプレイヤーを表示します。

## 動作上の注意

- 自動観戦は、制限を超えたプレイヤーが再スポーンした約1秒後に開始します。リスポーン自体を禁止する仕組みではありません。
- リスポーンする兵士・ロードアウト・場所はミッション側の設定に従います。観戦中の兵士を削除・非表示にする処理はありません。
- ヘリ搭乗対象や任務判定から観戦者を除外する処理は、組み込むミッション側でも確認してください。他Modの観戦判定との互換性は前提にしないでください。
- ローカルボイスの可聴範囲・音量・無線設定は変更しません。カメラ位置は0.5秒間隔でサーバーへ通知します。

## 作者

- [CatalpaBow](https://github.com/CatalpaBow)：CTA Spectatorの原作者。

## Contributors

- [CLY89](https://github.com/CLY89)：観戦カメラ位置でのローカルボイス受信機能、動作検証、READMEの整備。
