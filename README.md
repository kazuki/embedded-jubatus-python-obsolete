# embedded-jubatus-python

[jubatus_core](https://github.com/jubatus/jubatus_core)を直接Pythonから利用するためのラッパーライブラリ．
[Jubatusサーバ](https://github.com/jubatus/jubatus)不要で，より簡単にPythonからJubatusの機能を利用することが出来ます．

## 動作確認環境

* Python 2.7.10
* Python 3.4.3
* Python 3.5.1

## インストール方法＆テスト実行方法

インストール方法
```
$ python ./setup.py sdist
$ pip install dist/embedded-jubatus-*.tar.gz
```

テストの実行方法 (事前にインストールする必要があります)
```
$ mkdir jubatus
$ python ./setup.py nosetests
```

python ./setup.py install では上手くインストールできないので注意．
テストもインストールしてからじゃないと実行できませんし，
テスト実行時にビルドが走りますがテスト対象はインストールしたライブラリになります．

## 対応しているAPI

- [X] [Classifier](http://jubat.us/ja/api_classifier.html)
  - [X] train
  - [X] classify
  - [X] get_labels
  - [X] set_label
  - [X] delete_label
  - [X] load / dump / clear / get_config
- [X] [Regression](http://jubat.us/ja/api_regression.html)
  - [X] train
  - [X] estimate
  - [X] load / dump / clear / get_config
- [X] [Recommender](http://jubat.us/ja/api_recommender.html)
  - [X] clear_row
  - [X] update_row
  - [X] complete_row_from_id
  - [X] complete_row_from_datum
  - [X] similar_row_from_id
  - [X] similar_row_from_datum
  - [X] decode_row
  - [X] get_all_rows
  - [X] calc_similarity
  - [X] calc_l2norm
  - [X] load / dump / clear / get_config
- [X] [Nearest Neighbor](http://jubat.us/ja/api_nearest_neighbor.html)
  - [X] set_row
  - [X] neighbor_row_from_id
  - [X] similar_row_from_id
  - [X] similar_row_from_datum
  - [X] get_all_rows
  - [X] load / dump / clear / get_config
- [X] [Anomaly](http://jubat.us/ja/api_anomaly.html)
  - [X] add
  - [X] calc_score
  - [X] clear_row
  - [X] update
  - [X] overwrite
  - [X] get_all_rows
  - [X] load / dump / clear / get_config
- [X] [Clustering](http://jubat.us/ja/api_clustering.html)
  - [X] push
  - [X] get_revision
  - [X] get_core_members
  - [X] get_k_center
  - [X] get_nearest_center
  - [X] get_nearest_members
  - [X] load / dump / clear / get_config
- [X] [Burst](http://jubat.us/ja/api_burst.html)
  - [ ] add_documents
  - [ ] get_result
  - [ ] get_result_at
  - [ ] get_all_bursted_results
  - [ ] get_all_bursted_results_at
  - [ ] get_all_keywords
  - [ ] add_keyword
  - [ ] remove_keyword
  - [ ] remove_all_keywords
  - [X] load / dump / clear / get_config
- [X] [Bandit](http://jubat.us/ja/api_bandit.html)
  - [ ] register_arm
  - [ ] delete_arm
  - [ ] select_arm
  - [ ] register_reward
  - [ ] get_arm_info
  - [ ] reset
  - [X] load / dump / clear / get_config
- [ ] [Stat](http://jubat.us/ja/api_stat.html) (対応予定無し)
- [ ] [Graph](http://jubat.us/ja/api_graph.html) (対応予定無し)
