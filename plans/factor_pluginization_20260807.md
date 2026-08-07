# 因子插件化与真正 Pipeline 改造计划

日期：2026-08-07
状态：首期完成

## 当前现状

- [x] 已确认当前因子计算逻辑集中在 `FactorCalculator`
- [x] 已确认 `FactorPipelineEngine::worker_loop` 直接硬编码实例化计算器
- [x] 已确认输出结构 `FactorValue` 为固定字段结构体
- [x] 已确认配置仅覆盖 source / engine / publisher，尚无 factor/plugin 配置段
- [x] 已收到主人补充：目标不是“计算器可配置”，而是“真正的 pipeline”
- [x] 已确认：首期只做顶层 pipeline stage，worker 内部暂不拆子 pipeline
- [ ] 明确插件挂载在哪一层 pipeline
- [x] 已确认：首期先不做 worker 内部插件挂载
- [x] 已确认：首期不做新增插件配置格式，优先保留现有配置骨架并为顶层组件初始化服务
- [x] 已确认：输出继续兼容固定 `FactorValue`
- [x] 已补最小验证：`factor_compute_engine_test`
- [x] 已完成首期执行

## 已识别的核心约束

1. 当前名义上叫 pipeline，但本质是 `source -> hash dispatch -> worker compute -> sink publish` 的线程流水，不是“可编排的数据处理 pipeline”。
2. 当前 worker 阶段把“状态更新、因子计算、输出组装”耦合在一起，但主人已确认首期不拆它。
3. `FactorValue` 是定长结构体，天然不适合直接承载任意动态 schema。
4. worker 内部维护 `InstrumentState`，如果 pipeline stage 边界设计不稳，会破坏现有单合约状态更新路径和线程内局部性。
5. 共享内存下游如果已经依赖当前 `FactorValue` 布局，则不能直接做破坏性变更。

## 对“真正 Pipeline”的理解修正

“真正 pipeline”在当前已确认范围下，首期应至少具备以下特征：

1. 顶层阶段明确：`source -> dispatch -> worker -> publish`
2. 顶层 pipeline 负责组件装配和生命周期，而不是把所有逻辑揉进旧 `FactorPipelineEngine`
3. `Worker` 首期可以保持内部封装，不要求立刻变成可编排子 graph
4. 配置优先服务于顶层组件初始化，不强行引入因子节点配置

## 首期建议范围

1. 先做顶层“编译期模板式 pipeline”，不做 `.so` 动态装载。
2. `Worker` 内部首期不拆 stage，不做因子节点化。
3. 首期保持当前共享内存输出兼容，先不改下游消费协议。
4. 用最小可行方案验证：现有 4 个因子逻辑在新顶层 pipeline 下结果保持一致。

## 待主人确认的问题

1. 你说的“真正 pipeline”，更接近哪种目标
   - 方案 A：worker 内部多 stage 链式执行，每个 factor 是一个 stage 节点
   - [x] 方案 B：不仅 worker 内部，连 source / dispatch / publish 也都要抽象成统一 pipeline stage
2. 输出兼容策略
   - [x] 方案 A：继续输出固定 `FactorValue`
   - 方案 B：改成动态字段 / KV 输出
3. 首期目标粒度
   - [x] 方案 A：全链路顶层 stage 化，但内部保持现有 4 个因子实现
   - 方案 B：继续拆 worker 内部 pipeline / 新因子体系
4. 配置方式
   - [x] 方案 A：首期不引入 factor node 配置，优先保持顶层组件配置简洁
   - 方案 B：按节点对象配置，支持参数和顺序

## 预期执行步骤

1. [x] 定义顶层组件契约与模板化 `FactorComputeEngine`
2. [x] 抽出 `Source / Dispatcher / Worker / Publisher`
3. [x] 将当前 `FactorPipelineEngine` 迁移为模板 pipeline 驱动
4. [x] 保持 `Worker` 内部逻辑兼容现有 4 个因子
5. [x] 扩展 YAML 配置解析，适配新的顶层组件初始化
6. [x] 补最小验证用例，覆盖 pipeline 初始化与主链路结果
7. [x] 更新 README 与示例配置

## 已确认决策

1. 主人已确认：采用全链路统一 pipeline stage 方向，即 `source / dispatch / worker / publish` 都进入统一 pipeline 抽象。
2. 主人已指定参考 `../hft_md_gateway` 的模板 pipeline 设计。
3. 主人已确认：输出首期保持兼容，继续沿用固定 `FactorValue`。
4. 主人已确认：首期只承载现有 4 个因子。
5. 主人已确认：`worker` 内部可以先不做 pipeline。

## 参考模板结论：`hft_md_gateway`

已阅读核心文件：

1. `include/md_gateway/pipeline/market_data_pipeline.hpp`
2. `include/md_gateway/source/ctp_api_source.hpp`
3. `include/md_gateway/decoder/ctp_tick_record_decoder.hpp`
4. `include/md_gateway/publisher/shm_publisher.hpp`
5. `src/app/ctp_shm_demo.cpp`

其设计要点：

1. Pipeline 本身只负责装配和生命周期，不承载业务细节。
2. 每个组件通过约定接口接入：
   - `config_type`
   - `init(config)`
   - 上游组件产出 `output_type`
   - 中间组件暴露 `poll(...)` / `decode(...)`
   - 下游组件暴露 `publish(...)`
3. 通过模板参数固定主链路类型，例如 `Source -> Decoder -> Publisher...`
4. 配置加载由各组件各自实现 `load_component_config(...)`
5. app 入口极薄，只负责实例化具体 pipeline 并驱动 `run_once()`

## 映射到 `hft_factor` 的首期改造建议

要参考其“模板 pipeline 设计”，首期更合理的方向不是做运行时任意 stage 图，而是：

1. 先定义统一组件契约：
   - `Source`
   - `Dispatcher`
   - `Worker`
   - `Publisher`
2. 主 `FactorComputeEngine` 参考 `MarketDataPipeline`，只负责：
   - init
   - run_once / run
   - stop
   - 串接组件生命周期
3. `Worker` 首期保持内部封装，不强制再拆子 pipeline。
4. 因子插件化延后，先完成顶层 pipeline 模板化。

## 对当前目标的影响

这意味着首期方案先只做一层：

1. 顶层全链路模板 pipeline：
   - `Source -> Dispatcher -> Worker -> Publisher`
2. `Worker` 内部先维持现有实现风格：
   - 更新 `InstrumentState`
   - 调用当前因子计算逻辑
   - 产出固定 `FactorValue`

这样既满足主人要的“真正 pipeline 外壳”，又和 `hft_md_gateway` 的模板设计保持一致，也能控制改造范围。
