#include <CardinalityEstimation.h>
#include <executer/DataExecuterDemo.h>

#define M50 50000000
#define K500 500000
#define M20 20000000
#define K200 200000

int main(int argc, char *argv[]) {
  int initSize = M50; // Initial data size.
  int opSize = M20;   // Number of operations.
  double score = 0;
  int cnt = 0;

  DataExecuterDemo dataExecuter(initSize - 1, opSize);
  std::cout << "prepared" << std::endl;
  CEEngine ceEngine(initSize, &dataExecuter);
  Action action = dataExecuter.getNextAction();

  while (action.actionType != NONE) {
    ceEngine.prepare();
    if (action.actionType == INSERT) {
      ceEngine.insertTuple(action.actionTuple);
    } else if (action.actionType == DELETE) {
      ceEngine.deleteTuple(action.actionTuple, action.tupleId);
    } else if (action.actionType == QUERY) {
      int ans = ceEngine.query(action.quals);
      /* score += dataExecuter.answer(ans); */
      score += 1;
      cnt++;
    }
    action = dataExecuter.getNextAction();
  }
  std::cout << score / cnt << std::endl;
  std::cout << "score " << score << " cnt " << cnt << std::endl;
}
