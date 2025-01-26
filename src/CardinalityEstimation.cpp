//
// You should modify this file.
//
#include <CardinalityEstimation.h>
#include <cassert>
#include <common/Root.h>
#include <fstream>

static digestible::tdigest<int, long long int> digest_a(48);
static digestible::tdigest<int, long long int> digest_b(48);
static digestible::tdigest<int, long long int> digest_del_a(48);
static digestible::tdigest<int, long long int> digest_del_b(48);

uint64_t ins_counter = 0;
uint64_t del_counter = 0;
uint64_t counter = 1;

/* static uint64_t counter = 1; */
void CEEngine::insertTuple(const std::vector<int> &tuple) {
  counter++;
  ins_counter++;
  if (counter % (INIT_SIZE / SAMPLE_NUM) == 0) {
    /* std::cout << "call insert at: " << counter << std::endl; */
    digest_a.insert(tuple[0]);
    digest_b.insert(tuple[1]);

    topkInsert(tuple[0], topk_a, topk_a_sum);
    topkInsert(tuple[1], topk_b, topk_b_sum);
    sample_num_temp++;
  }
}

void CEEngine::deleteTuple(const std::vector<int> &tuple, int tupleId) {
  counter++;
  del_counter++;
  if (counter % (INIT_SIZE / SAMPLE_NUM) == 0) {
    if (const auto &search = topk_a.find(tuple[0]); search != topk_a.end()) {
      topk_a_sum--;
      if (search->second[1] == 1)
        topk_a.erase(search);
      else {
        search->second[1]--;
        search->second[0]--;
      }
    }
    if (const auto &search = topk_b.find(tuple[1]); search != topk_b.end()) {
      topk_b_sum--;
      if (search->second[1] == 1)
        topk_b.erase(search);
      else {
        search->second[1]--;
        search->second[0]--;
      }
    }

    sample_num_temp--;
    digest_del_a.insert(tuple[0]);
    digest_del_b.insert(tuple[1]);
    delete_num_temp++;
  }
}

void CEEngine::topkInsert(
    const int k, std::unordered_map<int, std::array<uint16_t, 2>> &topk,
    int &topk_sum) {
  if (topk.size() < TOPK_SIZE) {
    if (const auto &search = topk.find(k); search != topk.end()) { // contains
      search->second[0]++;
      search->second[1]++;
      topk_sum++;
    } else {
      topk[k] = {1, 1};
      topk_sum++;
    }
  } else {
    for (auto &i : topk) {
      i.second[0]--;
    }
    for (auto i = topk.begin(); i != topk.end();) {
      if (i->second[0] == 0) {
        topk_sum -= i->second[1];
        i = topk.erase(i);
      } else {
        i++;
      }
    }
    if (topk.size() < TOPK_SIZE) {
      topk[k] = {1, 1};
      topk_sum++;
    }
  }
}

int CEEngine::query(const std::vector<CompareExpression> &quals) {
  double prob = 1;
  /* bool eq = false; */
  for (auto &qual : quals) {
    if (qual.compareOp == EQUAL) {
      if (qual.columnIdx == 0) {        // A
        if (topk_a.count(qual.value)) { // contains
          /* std::cout << "topk_a: " << topk_a[qual.value][1] */
          /*           << " at: " << qual.value << std::endl; */
          prob *=
              (double)topk_a[qual.value][1] / (sample_num + sample_num_temp);
        } else
          prob = (double)(sample_num + sample_num_temp - topk_a_sum) /
                 ((digest_a.max() - digest_a.min() - topk_a.size()) *
                  (sample_num_temp + sample_num));
      } else {
        if (topk_b.count(qual.value)) { // contains
          /* std::cout << "topk_b: " << topk_a[qual.value][1] */
          /*           << " at: " << qual.value << std::endl; */
          prob *=
              (double)topk_b[qual.value][1] / (sample_num + sample_num_temp);
        } else
          prob = (double)(sample_num + sample_num_temp - topk_b_sum) /
                 ((digest_b.max() - digest_b.min() - topk_b.size()) *
                  (sample_num_temp + sample_num));
      }
    } else { // GREATER
      if (qual.columnIdx == 0) {
        prob *= (double)1 -
                ((double)(digest_a.cumulative_distribution(qual.value) *
                              sample_num -
                          digest_del_a.cumulative_distribution(qual.value) *
                              delete_num) /
                 (sample_num - delete_num));
        /* std::cout << prob << std::endl; */
      } else {
        prob *= (double)1 -
                ((double)(digest_b.cumulative_distribution(qual.value) *
                              sample_num -
                          digest_del_b.cumulative_distribution(qual.value) *
                              delete_num) /
                 (sample_num - delete_num));
        /* std::cout << prob << std::endl; */
      }
    }
  }
  return prob * (num + ins_counter - del_counter);
}

void CEEngine::prepare() {
  // Implement your prepare logic here.
  if (counter % (INIT_SIZE / SAMPLE_NUM * 100) == 0) {
    /* std::cout << "call prepare at: " << counter << std::endl; */
    digest_a.merge();
    digest_b.merge();
    digest_del_a.merge();
    digest_del_b.merge();
    delete_num += delete_num_temp;
    sample_num += sample_num_temp;
    delete_num_temp = 0;
    sample_num_temp = 0;
  }
}

CEEngine::CEEngine(int num, DataExecuter *dataExecuter) {
  // Implement your constructor here.
  /* std::cout << "init: " << getMemoryUsage() << std::endl; */
  this->dataExecuter = dataExecuter;
  this->num = num;

  std::vector<std::vector<int>> sample;
  for (int i = 0; i < 500; i++) {
    dataExecuter->readTuples(i * (num / 500), SAMPLE_NUM / 500, sample);
    for (const auto &tuple : sample) {
      digest_a.insert(tuple[0]);
      digest_b.insert(tuple[1]);
      topkInsert(tuple[0], topk_a, topk_a_sum);
      topkInsert(tuple[1], topk_b, topk_b_sum);
    }
    digest_a.merge();
    digest_b.merge();
    sample.clear();
  }
}
