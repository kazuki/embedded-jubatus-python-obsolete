# embedded-jubatus-python

[jubatus_core](https://github.com/jubatus/jubatus_core)を直接Pythonから利用するためのラッパーライブラリ．
[Jubatusサーバ](https://github.com/jubatus/jubatus)不要で，より簡単にPythonからJubatusの機能を利用することが出来ます．

## 動作確認環境

* Python 2.7.10
* Python 3.4.3

## 対応しているAPI

- [X] Classifier
  - [X] train
  - [X] classify
  - [X] get_labels
  - [X] set_label
  - [X] delete_label
  - [X] load / dump / clear / get_config
- [ ] Regression
  - [ ] train
  - [ ] estimate
  - [ ] load / dump / clear / get_config
- [X] Recommender
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
- [ ] Nearest Neighbor
  - [ ] set_row
  - [ ] neighbor_row_from_id
  - [ ] similar_row_from_id
  - [ ] similar_row_from_datum
  - [ ] get_all_rows
  - [ ] load / dump / clear / get_config
- [X] Anomaly
  - [X] add
  - [X] calc_score
  - [ ] clear_row
  - [ ] update
  - [ ] overwrite
  - [ ] get_all_rows
  - [ ] load / dump / clear / get_config
- [ ] Clustering
  - [ ] push
  - [ ] get_revision
  - [ ] get_core_members
  - [ ] get_k_center
  - [ ] get_nearest_center
  - [ ] get_nearest_members
  - [ ] load / dump / clear / get_config
- [ ] Burst
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
- [ ] Bandit
  - [ ] register_arm
  - [ ] delete_arm
  - [ ] select_arm
  - [ ] register_reward
  - [ ] get_arm_info
  - [ ] reset
  - [ ] load / dump / clear / get_config
- [ ] Stat (対応予定無し)
- [ ] Graph (対応予定無し)
