# TreeLox

Benchmark with clang release mode -O3 on WSL, Ryzen 7 4800H with 32? GB of total system RAM.

In seconds, `zoo_batch` num of batches

| Benchmark | jlox | treelox | clox |
| --- | --- | --- | --- |
| `binary_tree` | 6.8230 | 45.0520 | 2.9609 |
| `equality` | 0.6530 | 1.1080 | 0.0277 |
| `fib` | 3.8130 | 113.8270 | 1.1295 |
| `instantiation` | 1.1200 | 4.2220 | 0.4906 |
| `method_call` | 1.3960 | 16.5740 | 0.1676 |
| `properties` | 3.7560 | 55.0180 | 0.3925 |
| `string_equality` | 0.40 | 1.13 | ERR |
| `tree` | 22.3950 | 204.3030 | 2.1303 |
| `zoo` | 3.4510 | 37.4680 | 0.2964 |
| `zoo_batch` | 427 | 45 | 5250 |