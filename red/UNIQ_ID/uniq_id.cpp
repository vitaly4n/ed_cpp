
#define CONCAT(x, y) x##y
#define CONCAT_VALUES(x, y) CONCAT(x, y)
#define UNIQ_ID CONCAT_VALUES(val_, __LINE__)