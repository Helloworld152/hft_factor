# hft_factor

`hft_factor` 是一个基于共享内存的因子计算项目，用于从行情数据中计算因子并发布结果。

项目主链路由 4 个阶段组成：

- `Source`：从输入共享内存读取行情
- `Dispatcher`：按 `symbol` 将数据分发到固定 worker
- `Worker`：维护合约状态并执行因子计算
- `Publisher`：将计算结果写入输出共享内存

## 依赖

- `third_party/hft-common`：共享内存、队列和 factor 插件公共接口
- `third_party/yaml-cpp`：YAML 配置解析

## 内置因子

当前仓库内置了 4 个示例因子插件：

- `mid_price`
- `spread`
- `book_imbalance`
- `ret_1_tick`

## 因子插件 SDK

编写因子插件所需的公共头文件位于 `third_party/hft-common/include/hft_common/factor/`：

- `hft_common/factor/ctp_shm_tick_record.h`
- `hft_common/factor/instrument_state.h`
- `hft_common/factor/factor_context.h`
- `hft_common/factor/factor_plugin.h`

插件接口约定如下：

- 输入行情结构：`CtpShmTickRecord`
- 运行时状态结构：`InstrumentState`
- 插件上下文：`FactorContext`
- 插件抽象基类：`FactorNode`

其他项目如果要编写兼容当前 ABI 的因子插件，只需要依赖上述公共头文件，不需要依赖 `hft_factor` 的内部运行时目录。

## 构建

```bash
cmake -S . -B build
cmake --build build -j
```

## 运行

```bash
./bin/hft_factor ./conf/hft_factor.yaml
```

也可以直接使用：

```bash
./compile.sh
```

## 配置

示例配置：

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

字段说明：

1. `source.input_shm`：输入共享内存名
2. `engine.worker_count`：worker 线程数量
3. `engine.worker_queue_capacity`：`source -> worker` 队列容量，必须是 2 的幂
4. `engine.sink_queue_capacity`：`worker -> sink` 队列容量，必须是 2 的幂
5. `publisher.output_shm`：输出共享内存名
6. `publisher.output_capacity`：输出因子共享内存 ring 容量

## 目录结构

- `include/hft_factor/runtime/core`：配置与配置加载
- `include/hft_factor/runtime/components`：`Source / Dispatcher / Worker / Publisher`
- `include/hft_factor/runtime/engine`：`FactorComputeEngine`
- `include/hft_factor/runtime/internal`：运行时内部实现
- `src/plugins`：内置因子插件实现
- `conf`：示例配置
- `tools`：辅助工具

## 数据流

线程间队列使用 `hft-common` 的 `SpscQueue`：

- `source -> worker[i]`：单生产者单消费者队列
- `worker[i] -> publisher`：每个 worker 一条独立 SPSC，publisher 线程轮询汇总
