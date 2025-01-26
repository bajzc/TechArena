#ifndef CARDINALITYESTIMATION_ENGINE
#define CARDINALITYESTIMATION_ENGINE
//
// You should modify this file.
//
#include "digest.h"
#include <array>
#include <common/Expression.h>
#include <executer/DataExecuter.h>
#include <map>

#define TOTAL_OPERATION 20000000
#define INIT_SIZE 50000000
#define DATA_RANGE 20000000
#define SAMPLE_NUM 5000000
#define TOPK_SIZE 50000

size_t getMemoryUsage();

class CEEngine {
public:
  /**
   * Insert a tuple, indicating that the tuple is inserted and appended to the
   * end of the disk.
   * @param tuple Inserted tuple.
   */
  void insertTuple(const std::vector<int> &tuple);
  /**
   * Deletion function. Pass a tuple and tupleId, indicating that the tuple at
   * the tupleId position is deleted.
   * @param tuple Deleted tuple.
   * @param tupleId Location of the deleted tuple.
   */
  void deleteTuple(const std::vector<int> &tuple, int tupleId);
  /**
   * Query function, pass in expression, return estimated cardinality result.
   * @param quals expression.
   * @return return estimated cardinality result.
   */
  int query(const std::vector<CompareExpression> &quals);
  /**
   * Preprocessing function of the cardinality estimation algorithm. This
   * function is executed before each operation is called.
   */
  void prepare();

  void topkInsert(const int k,
                  std::unordered_map<int, std::array<uint16_t, 2>> &topk,
                  int &topk_sum);
  /**
   * The constructor function of cardinality estimation.
   * @param num Size of the initial data set.
   * @param dataExecuter Interfaces for datasets.
   */
  CEEngine(int num, DataExecuter *dataExecuter);
  ~CEEngine() = default;

private:
  DataExecuter *dataExecuter;
  int num;
  int sample_num_temp = 0;
  int delete_num_temp = 0;

  int topk_a_sum = 0;
  int topk_b_sum = 0;

  int sample_num = SAMPLE_NUM;
  int delete_num = 0;
  // array = {topk_counter, occur}
  std::unordered_map<int, std::array<uint16_t, 2>> topk_a;
  std::unordered_map<int, std::array<uint16_t, 2>> topk_b;
};
#endif
