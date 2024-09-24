
#include <vector>

using namespace std;
class Solution {
public:
    vector<int> findPeakGrid(vector<vector<int>>& mat) {
        int l = 0, r = mat.size() - 2;
        while (l < r) {
            int mid = l + (r - l) / 2;
            int big = -1;
            int bigI = -1;
            for (int i = 0; i < mat[mid].size(); i++) {
                if (mat[mid][i] > big) {
                    big = mat[mid][i];
                    bigI = i;
                }
            }
            if (big > mat[mid + 1][bigI]) {
                r = mid;
            } else {
                l = mid + 1;
            }
        }
        int big = -1;
        int bigI = -1;
        for (int i = 0; i < mat[l].size(); i++) {
            if (mat[l][i] > big) {
                big = mat[l][i];
                bigI = i;
            }
        }
        return {l, bigI};
    }
};
int main() {
    vector<vector<int>> mat = {{1, 2, 3, 4, 5, 6, 7, 8},
                               {2, 3, 4, 5, 6, 7, 8, 9},
                               {3, 4, 5, 6, 7, 8, 9, 10},
                               {4, 5, 6, 7, 8, 9, 10, 11}};
    Solution().findPeakGrid(mat);
    return 0;
}