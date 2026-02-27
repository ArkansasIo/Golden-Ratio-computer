#include "wolfman_alpha/wa_calc.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace wa::calc {

namespace {

class Parser {
public:
  Parser(std::string expr, double xValue) : s_(std::move(expr)), x_(xValue) {}

  double parse() {
    pos_ = 0;
    const double value = parseExpr();
    skipSpaces();
    if (pos_ != s_.size()) throw std::invalid_argument("Unexpected token near: " + s_.substr(pos_));
    return value;
  }

private:
  std::string s_;
  std::size_t pos_{0};
  double x_{0.0};

  void skipSpaces() {
    while (pos_ < s_.size() && std::isspace(static_cast<unsigned char>(s_[pos_]))) pos_++;
  }

  bool match(char c) {
    skipSpaces();
    if (pos_ < s_.size() && s_[pos_] == c) {
      pos_++;
      return true;
    }
    return false;
  }

  double parseExpr() {
    double lhs = parseTerm();
    while (true) {
      if (match('+')) lhs += parseTerm();
      else if (match('-')) lhs -= parseTerm();
      else break;
    }
    return lhs;
  }

  double parseTerm() {
    double lhs = parsePower();
    while (true) {
      if (match('*')) lhs *= parsePower();
      else if (match('/')) lhs /= parsePower();
      else break;
    }
    return lhs;
  }

  double parsePower() {
    double lhs = parseUnary();
    if (match('^')) {
      const double rhs = parsePower();
      return std::pow(lhs, rhs);
    }
    return lhs;
  }

  double parseUnary() {
    if (match('+')) return parseUnary();
    if (match('-')) return -parseUnary();
    return parsePrimary();
  }

  std::string parseIdentifier() {
    skipSpaces();
    const std::size_t start = pos_;
    if (pos_ < s_.size() && (std::isalpha(static_cast<unsigned char>(s_[pos_])) || s_[pos_] == '_')) {
      pos_++;
      while (pos_ < s_.size()) {
        const char ch = s_[pos_];
        if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_') pos_++;
        else break;
      }
      return s_.substr(start, pos_ - start);
    }
    return {};
  }

  double parseNumber() {
    skipSpaces();
    const std::size_t start = pos_;
    bool hasDot = false;
    while (pos_ < s_.size()) {
      const char ch = s_[pos_];
      if (std::isdigit(static_cast<unsigned char>(ch))) {
        pos_++;
      } else if (ch == '.' && !hasDot) {
        hasDot = true;
        pos_++;
      } else {
        break;
      }
    }
    if (start == pos_) throw std::invalid_argument("Expected number");
    return std::stod(s_.substr(start, pos_ - start));
  }

  static double applyFunction(const std::string& fn, double v) {
    if (fn == "sin") return std::sin(v);
    if (fn == "cos") return std::cos(v);
    if (fn == "tan") return std::tan(v);
    if (fn == "asin") return std::asin(v);
    if (fn == "acos") return std::acos(v);
    if (fn == "atan") return std::atan(v);
    if (fn == "sqrt") return std::sqrt(v);
    if (fn == "abs") return std::fabs(v);
    if (fn == "log") return std::log(v);
    if (fn == "log10") return std::log10(v);
    if (fn == "exp") return std::exp(v);
    if (fn == "floor") return std::floor(v);
    if (fn == "ceil") return std::ceil(v);
    throw std::invalid_argument("Unknown function: " + fn);
  }

  double parsePrimary() {
    skipSpaces();

    if (match('(')) {
      const double v = parseExpr();
      if (!match(')')) throw std::invalid_argument("Missing ')'");
      return v;
    }

    if (pos_ < s_.size() && (std::isalpha(static_cast<unsigned char>(s_[pos_])) || s_[pos_] == '_')) {
      std::string ident = parseIdentifier();
      std::transform(ident.begin(), ident.end(), ident.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

      if (ident == "x") return x_;
      if (ident == "pi") return 3.14159265358979323846;
      if (ident == "e") return 2.71828182845904523536;

      if (match('(')) {
        const double arg = parseExpr();
        if (!match(')')) throw std::invalid_argument("Missing ')' after function argument");
        return applyFunction(ident, arg);
      }
      throw std::invalid_argument("Unknown identifier: " + ident);
    }

    return parseNumber();
  }
};

double eval_residual(const std::string& lhs, const std::string& rhs, double x) {
  return eval_expr(lhs, x) - eval_expr(rhs, x);
}

} // namespace

double eval_expr(const std::string& expr, double xValue) {
  Parser p(expr, xValue);
  return p.parse();
}

double derivative(const std::string& expr, double xValue, double h) {
  if (h <= 0.0) throw std::invalid_argument("h must be > 0");
  const double fwd = eval_expr(expr, xValue + h);
  const double bwd = eval_expr(expr, xValue - h);
  return (fwd - bwd) / (2.0 * h);
}

double integrate(const std::string& expr, double a, double b, int steps) {
  if (steps < 2) steps = 2;
  if (steps % 2 != 0) steps++;
  const double h = (b - a) / static_cast<double>(steps);
  double sum = eval_expr(expr, a) + eval_expr(expr, b);
  for (int i = 1; i < steps; ++i) {
    const double x = a + static_cast<double>(i) * h;
    sum += ((i % 2) ? 4.0 : 2.0) * eval_expr(expr, x);
  }
  return sum * (h / 3.0);
}

QuadraticResult solve_quadratic(double a, double b, double c) {
  const double eps = 1e-12;
  QuadraticResult out{};

  if (std::fabs(a) < eps) {
    if (std::fabs(b) < eps) {
      out.rootCount = 0;
      return out;
    }
    out.rootCount = 1;
    out.x1 = -c / b;
    out.x2 = out.x1;
    return out;
  }

  const double d = b * b - 4.0 * a * c;
  if (d > eps) {
    const double sd = std::sqrt(d);
    out.realRoots = true;
    out.rootCount = 2;
    out.x1 = (-b + sd) / (2.0 * a);
    out.x2 = (-b - sd) / (2.0 * a);
  } else if (std::fabs(d) <= eps) {
    out.realRoots = true;
    out.rootCount = 1;
    out.x1 = -b / (2.0 * a);
    out.x2 = out.x1;
  } else {
    out.realRoots = false;
    out.rootCount = 2;
    out.x1 = -b / (2.0 * a);
    out.x2 = out.x1;
    out.imag = std::sqrt(-d) / (2.0 * a);
  }
  return out;
}

std::string trim_copy(const std::string& s) {
  const auto wsfront = std::find_if_not(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); });
  const auto wsback = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char c) { return std::isspace(c); }).base();
  if (wsfront >= wsback) return {};
  return std::string(wsfront, wsback);
}

std::vector<std::string> split_equation(const std::string& equation) {
  const auto pos = equation.find('=');
  if (pos == std::string::npos) throw std::invalid_argument("Equation must contain '='");
  const std::string lhs = trim_copy(equation.substr(0, pos));
  const std::string rhs = trim_copy(equation.substr(pos + 1));
  if (lhs.empty() || rhs.empty()) throw std::invalid_argument("Equation sides cannot be empty");
  return {lhs, rhs};
}

LinearEquationResult solve_linear_equation(const std::string& equation) {
  const auto sides = split_equation(equation);
  const std::string& lhs = sides[0];
  const std::string& rhs = sides[1];

  const double f0 = eval_residual(lhs, rhs, 0.0);
  const double f1 = eval_residual(lhs, rhs, 1.0);
  const double a = f1 - f0;
  const double b = f0;
  const double eps = 1e-10;

  if (std::fabs(a) < eps) {
    if (std::fabs(b) < eps) return {LinearSolveKind::InfiniteSolutions, 0.0};
    return {LinearSolveKind::NoSolution, 0.0};
  }
  return {LinearSolveKind::OneSolution, -b / a};
}

} // namespace wa::calc
