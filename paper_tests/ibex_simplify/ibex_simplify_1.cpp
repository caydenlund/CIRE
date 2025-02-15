#include <ibex.h>
#include <iostream>

using namespace ibex;

int main() {
  // Define variables
  Variable m0, m1, m2, w0, w1, w2, a0, a1, a2;
  Array<const ExprSymbol> symbols;
  symbols.add(m0);
  symbols.add(m1);
  symbols.add(m2);
  symbols.add(w0);
  symbols.add(w1);
  symbols.add(w2);
  symbols.add(a0);
  symbols.add(a1);
  symbols.add(a2);

  // Define the function r using IBEX's Function class
  Function r(
          {m0, m1, m2, w0, w1, w2, a0, a1, a2},
          (0.0 +
           ((((w2 * (0.0 - m2)) * (-3.0 * ((1.0 * (a2/w2)) * (a2/w2)))) * 1.0) +
            ((((w1 * (0.0 - m1)) * (-3.0 * ((1.0 * (a1/w1)) * (a1/w1)))) * 1.0) +
            ((((w0 * (0.0 - m0)) * (-3.0 * ((1.0 * (a0/w0)) * (a0/w0)))) * 1.0) + 0.0)))).simplify(0));

  // Define input values for evaluation
  IntervalVector input({{-1.0, 1.0},
                            {-1.0, 1.0},
                            {-1.0, 1.0},
                            {0.00001, 1.0},
                            {0.00001, 1.0},
                            {0.00001, 1.0},
                            {0.00001, 1.0},
                            {0.00001, 1.0},
                            {0.00001, 1.0}});

  // Evaluate function
  Interval result = r.eval(input);

  // Print result
  std::cout << "Computed result: " << result << std::endl;


  std::cout << -r.expr() << std::endl;

  SystemFactory factory;
  factory.add_var(symbols);
  factory.add_goal((r.expr()));
  System system(factory);

  DefaultOptimizerConfig optConfig(system,
                                   ibex::OptimizerConfig::default_rel_eps_f,
                                   ibex::OptimizerConfig::default_abs_eps_f,
                                   ibex::NormalizedSystem::default_eps_h,
//                                         1e-01,
//                                         1e-01,
//                                         1e-01,
                                   false,
                                   ibex::DefaultOptimizerConfig::default_inHC4,
                                   false,
                                   ibex::DefaultOptimizerConfig::default_random_seed,
                                   ibex::OptimizerConfig::default_eps_x);

  Optimizer opt(optConfig);

  opt.optimize(input);

  opt.report();
  std::cout << "Optimum point: " << ibex::Interval(opt.get_uplo(), opt.get_loup()) << std::endl;

  return 0;
}
