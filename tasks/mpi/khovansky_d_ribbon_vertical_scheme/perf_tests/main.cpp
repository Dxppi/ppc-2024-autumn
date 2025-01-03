// Copyright 2024 Khovansky Dmitry
#include <gtest/gtest.h>

#include <boost/mpi.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/timer.hpp>
#include <memory>
#include <random>
#include <vector>

#include "core/perf/include/perf.hpp"
#include "mpi/khovansky_d_ribbon_vertical_scheme/include/ops_mpi.hpp"

TEST(khovansky_d_ribbon_vertical_scheme_mpi, Performance_Pipeline_Run) {
  boost::mpi::environment env;
  boost::mpi::communicator world;

  std::vector<int> input_matrix;
  std::vector<int> input_vector;
  std::vector<int> output_vector;

  std::shared_ptr<ppc::core::TaskData> taskDataPar = std::make_shared<ppc::core::TaskData>();
  int rows_count;
  int columns_count;

  if (world.rank() == 0) {
    rows_count = 8192;
    columns_count = 8192;

    input_vector.resize(columns_count);
    input_matrix.resize(rows_count * columns_count);
    output_vector.resize(columns_count, 0);

    for (int i = 0; i < rows_count * columns_count; i++) {
      input_matrix[i] = (rand() % 1000) - 500;
      if (i < rows_count) {
        input_vector[i] = (rand() % 1000) - 500;
      }
    }

    taskDataPar->inputs.emplace_back(reinterpret_cast<uint8_t*>(input_matrix.data()));
    taskDataPar->inputs_count.emplace_back(input_matrix.size());

    taskDataPar->inputs.emplace_back(reinterpret_cast<uint8_t*>(input_vector.data()));
    taskDataPar->inputs_count.emplace_back(input_vector.size());

    taskDataPar->outputs.emplace_back(reinterpret_cast<uint8_t*>(output_vector.data()));
    taskDataPar->outputs_count.emplace_back(output_vector.size());
  }

  auto taskParallel = std::make_shared<khovansky_d_ribbon_vertical_scheme_mpi::RibbonVerticalSchemeMPI>(taskDataPar);
  ASSERT_TRUE(taskParallel->validation());
  taskParallel->pre_processing();
  taskParallel->run();
  taskParallel->post_processing();

  auto perfAttr = std::make_shared<ppc::core::PerfAttr>();
  perfAttr->num_running = 10;
  const boost::mpi::timer current_timer;
  perfAttr->current_timer = [&] { return current_timer.elapsed(); };

  auto perfResults = std::make_shared<ppc::core::PerfResults>();

  auto perfAnalyzer = std::make_shared<ppc::core::Perf>(taskParallel);
  perfAnalyzer->pipeline_run(perfAttr, perfResults);

  if (world.rank() == 0) {
    ppc::core::Perf::print_perf_statistic(perfResults);

    std::vector<int> seq_result(output_vector.size(), 0);

    auto taskDataSeq = std::make_shared<ppc::core::TaskData>();

    taskDataSeq->inputs.emplace_back(reinterpret_cast<uint8_t*>(input_matrix.data()));
    taskDataSeq->inputs_count.emplace_back(input_matrix.size());
    taskDataSeq->inputs.emplace_back(reinterpret_cast<uint8_t*>(input_vector.data()));
    taskDataSeq->inputs_count.emplace_back(input_vector.size());
    taskDataSeq->outputs.emplace_back(reinterpret_cast<uint8_t*>(seq_result.data()));
    taskDataSeq->outputs_count.emplace_back(seq_result.size());

    auto taskSequential =
        std::make_shared<khovansky_d_ribbon_vertical_scheme_mpi::RibbonVerticalSchemeSeq>(taskDataSeq);
    ASSERT_TRUE(taskSequential->validation());
    ASSERT_TRUE(taskSequential->pre_processing());
    ASSERT_TRUE(taskSequential->run());
    ASSERT_TRUE(taskSequential->post_processing());
    ASSERT_EQ(output_vector, seq_result);
  }
}

TEST(khovansky_d_ribbon_vertical_scheme_mpi, Performance_Task_Run) {
  boost::mpi::environment env;
  boost::mpi::communicator world;

  int rows_count;
  int columns_count;
  std::vector<int> input_matrix;
  std::vector<int> input_vector;
  std::vector<int> output_vector;

  std::shared_ptr<ppc::core::TaskData> taskDataPar = std::make_shared<ppc::core::TaskData>();

  if (world.rank() == 0) {
    rows_count = 8192;
    columns_count = 8192;

    input_matrix.resize(rows_count * columns_count);
    input_vector.resize(columns_count);
    output_vector.resize(columns_count, 0);

    for (int i = 0; i < rows_count * columns_count; i++) {
      input_matrix[i] = (rand() % 1000) - 500;
      if (i < rows_count) {
        input_vector[i] = (rand() % 1000) - 500;
      }
    }

    taskDataPar->inputs.emplace_back(reinterpret_cast<uint8_t*>(input_matrix.data()));
    taskDataPar->inputs_count.emplace_back(input_matrix.size());
    taskDataPar->inputs.emplace_back(reinterpret_cast<uint8_t*>(input_vector.data()));
    taskDataPar->inputs_count.emplace_back(input_vector.size());
    taskDataPar->outputs.emplace_back(reinterpret_cast<uint8_t*>(output_vector.data()));
    taskDataPar->outputs_count.emplace_back(output_vector.size());
  }

  auto taskParallel = std::make_shared<khovansky_d_ribbon_vertical_scheme_mpi::RibbonVerticalSchemeMPI>(taskDataPar);
  ASSERT_TRUE(taskParallel->validation());
  ASSERT_TRUE(taskParallel->pre_processing());
  ASSERT_TRUE(taskParallel->run());
  ASSERT_TRUE(taskParallel->post_processing());

  auto perfAttr = std::make_shared<ppc::core::PerfAttr>();
  perfAttr->num_running = 10;
  const boost::mpi::timer current_timer;
  perfAttr->current_timer = [&] { return current_timer.elapsed(); };

  auto perfResults = std::make_shared<ppc::core::PerfResults>();

  auto perfAnalyzer = std::make_shared<ppc::core::Perf>(taskParallel);
  perfAnalyzer->task_run(perfAttr, perfResults);

  if (world.rank() == 0) {
    ppc::core::Perf::print_perf_statistic(perfResults);

    std::vector<int> seq_result(output_vector.size(), 0);

    auto taskDataSeq = std::make_shared<ppc::core::TaskData>();
    taskDataSeq->inputs.emplace_back(reinterpret_cast<uint8_t*>(input_matrix.data()));
    taskDataSeq->inputs_count.emplace_back(input_matrix.size());
    taskDataSeq->inputs.emplace_back(reinterpret_cast<uint8_t*>(input_vector.data()));
    taskDataSeq->inputs_count.emplace_back(input_vector.size());
    taskDataSeq->outputs.emplace_back(reinterpret_cast<uint8_t*>(seq_result.data()));
    taskDataSeq->outputs_count.emplace_back(seq_result.size());

    auto taskSequential =
        std::make_shared<khovansky_d_ribbon_vertical_scheme_mpi::RibbonVerticalSchemeSeq>(taskDataSeq);
    ASSERT_TRUE(taskSequential->validation());
    ASSERT_TRUE(taskSequential->pre_processing());
    ASSERT_TRUE(taskSequential->run());
    ASSERT_TRUE(taskSequential->post_processing());
    ASSERT_EQ(output_vector, seq_result);
  }
}