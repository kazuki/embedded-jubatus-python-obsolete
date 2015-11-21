# embedded-jubatus-python

[jubatus_core](https://github.com/jubatus/jubatus_core)を直接Pythonから利用するためのラッパーライブラリ．
[Jubatusサーバ](https://github.com/jubatus/jubatus)不要で，より簡単にPythonからJubatusの機能を利用することが出来ます．

## 動作確認環境

* Python 2.7.10
* Python 3.4.3

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
  - [ ] load / dump / clear / get_config
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
  - [ ] load / dump / clear / get_config
- [X] [Nearest Neighbor](http://jubat.us/ja/api_nearest_neighbor.html)
  - [ ] set_row
  - [ ] neighbor_row_from_id
  - [ ] similar_row_from_id
  - [ ] similar_row_from_datum
  - [ ] get_all_rows
  - [ ] load / dump / clear / get_config
- [X] [Anomaly](http://jubat.us/ja/api_anomaly.html)
  - [X] add
  - [X] calc_score
  - [ ] clear_row
  - [ ] update
  - [ ] overwrite
  - [ ] get_all_rows
  - [ ] load / dump / clear / get_config
- [X] [Clustering](http://jubat.us/ja/api_clustering.html)
  - [ ] push
  - [ ] get_revision
  - [ ] get_core_members
  - [ ] get_k_center
  - [ ] get_nearest_center
  - [ ] get_nearest_members
  - [ ] load / dump / clear / get_config
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
  - [ ] load / dump / clear / get_config
- [X] [Bandit](http://jubat.us/ja/api_bandit.html)
  - [ ] register_arm
  - [ ] delete_arm
  - [ ] select_arm
  - [ ] register_reward
  - [ ] get_arm_info
  - [ ] reset
  - [ ] load / dump / clear / get_config
- [ ] [Stat](http://jubat.us/ja/api_stat.html) (対応予定無し)
- [ ] [Graph](http://jubat.us/ja/api_graph.html) (対応予定無し)
