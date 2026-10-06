#define CATCH_CONFIG_RUNNER
#include <cmd_options.hh>
#include <hip_test_common.hh>
#include <iostream>

CmdOptions cmd_options;

int main(int argc, char** argv) {
  Catch::Session session;

  using namespace Catch::clara;
  // clang-format off
  auto cli = session.cli()
    | Opt(cmd_options.iterations, "iterations")
        ["-I"]["--iterations"]
        ("Number of iterations used for performance tests (default: 1000)")
    | Opt(cmd_options.warmups, "warmups")
        ["-W"]["--warmups"]
        ("Number of warmup iterations used for performance tests (default: 100)")
    | Opt(cmd_options.no_display)
        ["-S"]["--no-display"]
        ("Do not display the output of performance tests")
    | Opt(cmd_options.progress)
        ["-P"]["--progress"]
        ("Show progress bar when running performance tests")
    | Opt(cmd_options.cg_extended_run, "cg_extened_run")
        ["-E"]["--cg-extended-run"]
        ("TODO: Description goes here")
    | Opt(cmd_options.cg_iterations, "cg_iterations")
        ["-C"]["--cg-iterations"]
        ("Number of iterations used for cooperative groups sync tests (default: 5)")
    | Opt(cmd_options.accuracy_iterations, "accuracy_iterations")
        ["-A"]["--accuracy-iterations"]
        ("Number of iterations used for math accuracy tests with randomly generated inputs (default: 2^32)")
    | Opt(cmd_options.accuracy_max_memory, "accuracy_max_memory")
        ["-M"]["--accuracy-max-memory"]
        ("Percentage of global device memory allowed for math accuracy tests (default: 80%)")
  ;
  // clang-format on

  session.cli(cli);

  int out = session.applyCommandLine(argc, argv);
  if (out != 0) return out;

  auto& context = TestContext::get(argc, argv);
  // A lone single-filter spec is matched as is. Otherwise "" is matched, as for the old
  // argc != 2, and a lone multi-filter spec such as "a,b" must also match as a whole.
  const auto& cfg = session.configData();
  const auto& spec = cfg.testsOrTags;
  bool single = spec.size() == 1 &&
      Catch::parseTestSpec(Catch::trim(spec[0])).matchesByFilter({}, Catch::Config()).size() == 1;
  if (!cfg.showHelp && !cfg.libIdentify && !cfg.listTests && !cfg.listTags && !cfg.listReporters &&
      !cfg.listTestNamesOnly &&
      (single ? context.skipTest(spec[0])
              : context.skipTest("") && (spec.size() != 1 || context.skipTest(spec[0])))) {
    // CTest uses this regex to figure out if the test has been skipped
    std::cout << "HIP_SKIP_THIS_TEST" << std::endl;
    return 0;
  }

  out = session.run();
  TestContext::get().cleanContext();
  return out;
}
