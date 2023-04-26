# TreeLox

`dart tool/bin/test.dart jlox --interpreter ../build/lox`

`dart tool/bin/benchmark.dart ./jlox ./treelox ./clox binary_tree`

Benchmark with clang release mode -O3 on WSL, Ryzen 7 4800H with 32? GB of total system RAM.

In seconds, `zoo_batch` num of batches

| Benchmark | treelox | jlox | clox |
| --- | --- | --- | --- |
| `binary_trees` | 45.0520 | 6.8230 | 2.9609 |
| `equality` | 1.1080 | 0.6530 | 0.0277 |
| `fib` | 113.8270 | 3.8130 | 1.1295 |
| `instantiation` | 4.2220 | 1.1200 | 0.4906 |
| `method_call` | 16.5740 | 1.3960 | 0.1676 |
| `properties` | 55.0180 | 3.7560 | 0.3925 |
| `string_equality` | 1.13 | 0.40 | ERR |
| `trees` | 204.3030 | 22.3950 | 2.1303 |
| `zoo` | 37.4680 | 3.4510 | 0.2964 |
| `zoo_batch` | 45 | 427 | 5250 |

With interning variable depth:

| Benchmark | treelox |
| --- | --- |
| `binary_trees `   | 43.2720 | 
| `equality`        | 1.0890 |
| `fib`             | 114.1520 |
| `instantiation`   | 4.046 |
| `method_call`     | 16.30 |
| `properties`      | 54.79 |
| `string_equality` | 1.12 |
| `trees`           | 200.93 |
| `zoo`             | 36.73 |
| `zoo_batch`       | 45 |

Using global instead of exceptions for return

| Benchmark | treelox |
| --- | --- |
| `binary_trees `   | 13.30
| `equality`        | 0.89
| `fib`             | 10.62
| `instantiation`   | 4.21
| `method_call`     | 1.96
| `properties`      | 5.62
| `string_equality` | 1.02
| `trees`           | 25.51
| `zoo`             | 3.94
| `zoo_batch`       | 404
