# hft_factor

独立的共享内存因子计算项目，采用分层 pipeline 架构。

当前依赖：

- `third_party/hft-common`：项目自带子仓库，提供共享内存 ring 和基础组件
- `third_party/yaml-cpp`：项目自带子仓库，用于 YAML 配置解析

运行时主链路：

- `RawTickShmSource`：读取 `CTP_MD` 原始行情共享内存
- `Dispatcher`：按 `symbol` 哈希到固定 worker
- `Worker`：维护分片内合约状态并计算基础因子
- `Sink`：单线程写入 `FACTOR_MD` 共享内存

当前已实现的基础因子：

- `mid_price`
- `spread`
- `book_imbalance`
- `ret_1_tick`

## 构建

```bash
cmake -S /home/ruanying/hft_factor -B /home/ruanying/hft_factor/build
cmake --build /home/ruanying/hft_factor/build -j
```

## 运行

```bash
/home/ruanying/hft_factor/bin/hft_factor_demo /home/ruanying/hft_factor/conf/hft_factor.yaml
```

配置文件示例：

```yaml
source:
  input_shm: CTP_MD

engine:
  worker_count: 4
  worker_queue_capacity: 4096
  sink_queue_capacity: 4096

publisher:
  output_shm: FACTOR_MD
  output_capacity: 1048576
```

字段含义：

1. `source.input_shm`：输入共享内存名
2. `engine.worker_count`：worker 线程数量
3. `engine.worker_queue_capacity`：`source -> worker` 队列容量，必须是 2 的幂
4. `engine.sink_queue_capacity`：`worker -> sink` 队列容量，必须是 2 的幂
5. `publisher.output_shm`：输出共享内存名
6. `publisher.output_capacity`：输出因子共享内存 ring 容量

## 当前实现说明

当前线程间队列已经切到 `hft-common` 里的进程内固定容量 `SpscQueue`：

- `source -> worker[i]`：单生产者单消费者
- `worker[i] -> sink`：每个 worker 一条独立 SPSC，sink 线程轮询汇总
