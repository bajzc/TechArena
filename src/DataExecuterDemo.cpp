//
// Demo data generator for local debugging. You can implement your own data
// generator for debugging based on this class.
//

#include <executer/DataExecuterDemo.h>

#define DATA_RANGE 20000000

std::unordered_map<int, bool> vis;
DataExecuterDemo::DataExecuterDemo(int end, int count) : DataExecuter() {
  this->end = end;
  this->count = count;
  for (int i = 0; i <= end; ++i) {
    std::vector<int> tuple;
    tuple.push_back((int)(sqrt(DATA_RANGE) * sqrt(rand())) % DATA_RANGE);
    tuple.push_back((int)(sqrt(DATA_RANGE) * sqrt(rand())) % DATA_RANGE);
    set.push_back(tuple);
  }
}

std::vector<int> DataExecuterDemo::generateInsert() {
  std::vector<int> tuple;
  tuple.push_back(rand() % DATA_RANGE);
  tuple.push_back(rand() % DATA_RANGE);
  set.push_back(tuple);
  end++;
  return tuple;
}

int DataExecuterDemo::generateDelete() {
  int x = (rand()) % end;
  while (vis[x]) {
    x = (rand()) % end;
  }
  vis[x] = true;
  return x;
}

void DataExecuterDemo::readTuples(int start, int offset,
                                  std::vector<std::vector<int>> &vec) {
  for (int i = start; i < start + offset; ++i) {
    if (!vis[i]) {
      vec.push_back(set[i]);
    }
  }
  return;
};

Action DataExecuterDemo::getNextAction() {
  Action action;
  if (count == 0) {
    action.actionType = NONE;
    return action;
  }
  if (count % 100 == 99) {
    action.actionType = QUERY;
    CompareExpression expr = {rand() % 2, CompareOp(rand() % 2),
                              rand() % DATA_RANGE};
    action.quals.push_back(expr);
  } else if (count % 100 < 90) {
    action.actionType = INSERT;
    action.actionTuple = generateInsert();
  } else {
    action.actionType = DELETE;
    action.tupleId = generateDelete();
    action.actionTuple = set[action.tupleId];
  }
  count--;
  curAction = action;
  return action;
};

double DataExecuterDemo::answer(int ans) {
  int cnt = 0;
  for (int i = 0; i <= end; ++i) {
    if (vis[i])
      continue;
    bool flag = true;
    for (int j = 0; j < curAction.quals.size(); ++j) {
      CompareExpression &expr = curAction.quals[j];
      if (expr.compareOp == GREATER && set[i][expr.columnIdx] <= expr.value) {
        flag = false;
        break;
      }
      if (expr.compareOp == EQUAL && set[i][expr.columnIdx] != expr.value) {
        flag = false;
        break;
      }
    }
    if (flag)
      cnt++;
  }
  double error = fabs(std::log((ans + 1) * 1.0 / (cnt + 1)));
  /* std::cout << "ans: " << ans << " actual: " << cnt << std::endl; */
  /* if (isnanf(error)) { */
  /*   std::cout << ans << std::endl; */
  /*   std::exit(-1); */
  /* } */
  return error;
};
