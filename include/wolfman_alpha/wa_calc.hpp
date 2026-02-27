#pragma once
#include <string>
#include <vector>

namespace wa::calc {

double eval_expr(const std::string& expr, double xValue = 0.0);
double derivative(const std::string& expr, double xValue, double h = 1e-5);
double integrate(const std::string& expr, double a, double b, int steps = 1000);

struct QuadraticResult {
  bool realRoots{true};
  int rootCount{0};
  double x1{0.0};
  double x2{0.0};
  double imag{0.0};
};

QuadraticResult solve_quadratic(double a, double b, double c);

enum class LinearSolveKind {
  OneSolution,
  NoSolution,
  InfiniteSolutions
};

struct LinearEquationResult {
  LinearSolveKind kind{LinearSolveKind::NoSolution};
  double x{0.0};
};

LinearEquationResult solve_linear_equation(const std::string& equation);

std::string trim_copy(const std::string& s);
std::vector<std::string> split_equation(const std::string& equation);

} // namespace wa::calc
