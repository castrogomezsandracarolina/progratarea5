#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

class Expression {
public:
    virtual ~Expression() = default;
    virtual double evaluate(double x) const = 0;
    virtual std::shared_ptr<Expression> derivative() const = 0;
    virtual std::string toString() const = 0;
    virtual bool constantValue(double& value) const {
        (void)value;
        return false;
    }
};

using Expr = std::shared_ptr<Expression>;
class Constant : public Expression {
private:
    double value_;
public:
    explicit Constant(double value) : value_(value) {}
    double evaluate(double) const override { return value_; }
    Expr derivative() const override { return std::make_shared<Constant>(0); }
    std::string toString() const override {
        std::ostringstream out;
        out << value_;
        return out.str();
    }
    bool constantValue(double& value) const override {
        value = value_;
        return true;
    }
};

class Variable : public Expression {
public:
    double evaluate(double x) const override { return x; }
    Expr derivative() const override { return std::make_shared<Constant>(1); }
    std::string toString() const override { return "x"; }
};

class BinaryOperation : public Expression {
protected:
    Expr left_;
    Expr right_;
public:
    BinaryOperation(Expr left, Expr right)
        : left_(std::move(left)), right_(std::move(right)) {}
};

class UnaryFunction : public Expression {
protected:
    Expr operand_;
public:
    explicit UnaryFunction(Expr operand) : operand_(std::move(operand)) {}
};

class Add : public BinaryOperation {
public:
    using BinaryOperation::BinaryOperation;
    double evaluate(double x) const override {
        return left_->evaluate(x) + right_->evaluate(x);
    }
    Expr derivative() const override;
    std::string toString() const override {
        return "(" + left_->toString() + " + " + right_->toString() + ")";
    }
    bool constantValue(double& value) const override {
        double a, b;
        if (!left_->constantValue(a) || !right_->constantValue(b)) return false;
        value = a + b;
        return true;
    }
};

class Multiply : public BinaryOperation {
public:
    using BinaryOperation::BinaryOperation;
    double evaluate(double x) const override {
        return left_->evaluate(x) * right_->evaluate(x);
    }
    Expr derivative() const override;
    std::string toString() const override {
        return "(" + left_->toString() + " * " + right_->toString() + ")";
    }
    bool constantValue(double& value) const override {
        double a, b;
        if (!left_->constantValue(a) || !right_->constantValue(b)) return false;
        value = a * b;
        return true;
    }
};

class Divide : public BinaryOperation {
public:
    using BinaryOperation::BinaryOperation;
    double evaluate(double x) const override {
        return left_->evaluate(x) / right_->evaluate(x);
    }
    Expr derivative() const override;
    std::string toString() const override {
        return "(" + left_->toString() + " / " + right_->toString() + ")";
    }
};

class Power : public BinaryOperation {
public:
    using BinaryOperation::BinaryOperation;
    double evaluate(double x) const override {
        return std::pow(left_->evaluate(x), right_->evaluate(x));
    }
    Expr derivative() const override;
    std::string toString() const override {
        return "(" + left_->toString() + " ^ " + right_->toString() + ")";
    }
};

class Sin : public UnaryFunction {
public:
    using UnaryFunction::UnaryFunction;
    double evaluate(double x) const override {
        return std::sin(operand_->evaluate(x));
    }
    Expr derivative() const override;
    std::string toString() const override {
        return "sin(" + operand_->toString() + ")";
    }
};

class Cos : public UnaryFunction {
public:
    using UnaryFunction::UnaryFunction;
    double evaluate(double x) const override {
        return std::cos(operand_->evaluate(x));
    }
    Expr derivative() const override;
    std::string toString() const override {
        return "cos(" + operand_->toString() + ")";
    }
};

class Ln : public UnaryFunction {
public:
    using UnaryFunction::UnaryFunction;
    double evaluate(double x) const override {
        return std::log(operand_->evaluate(x));
    }
    Expr derivative() const override;
    std::string toString() const override {
        return "ln(" + operand_->toString() + ")";
    }
};

Expr Add::derivative() const {
    return std::make_shared<Add>(left_->derivative(), right_->derivative());
}

Expr Multiply::derivative() const {
    return std::make_shared<Add>(
        std::make_shared<Multiply>(left_->derivative(), right_),
        std::make_shared<Multiply>(left_, right_->derivative()));
}

Expr Divide::derivative() const {
    return std::make_shared<Divide>(
        std::make_shared<Add>(
            std::make_shared<Multiply>(left_->derivative(), right_),
            std::make_shared<Multiply>(
                std::make_shared<Constant>(-1),
                std::make_shared<Multiply>(left_, right_->derivative()))),
        std::make_shared<Power>(right_, std::make_shared<Constant>(2)));
}

Expr Power::derivative() const {
    double exponent;
    if (right_->constantValue(exponent)) {
        if (exponent == 0) return std::make_shared<Constant>(0);
        if (exponent == 1) return left_->derivative();
        return std::make_shared<Multiply>(
            std::make_shared<Multiply>(
                std::make_shared<Constant>(exponent),
                std::make_shared<Power>(left_, std::make_shared<Constant>(exponent - 1))),
            left_->derivative());
    }
    return std::make_shared<Multiply>(
        std::make_shared<Power>(left_, right_),
        std::make_shared<Add>(
            std::make_shared<Multiply>(right_->derivative(),
                                       std::make_shared<Ln>(left_)),
            std::make_shared<Multiply>(right_,
                std::make_shared<Divide>(left_->derivative(), left_))));
}

Expr Sin::derivative() const {
    return std::make_shared<Multiply>(
        std::make_shared<Cos>(operand_), operand_->derivative());
}

Expr Cos::derivative() const {
    return std::make_shared<Multiply>(
        std::make_shared<Multiply>(
            std::make_shared<Constant>(-1), std::make_shared<Sin>(operand_)),
        operand_->derivative());
}

Expr Ln::derivative() const {
    return std::make_shared<Divide>(operand_->derivative(), operand_);
}

int main() {
    auto x = std::make_shared<Variable>();
    Expr expr = std::make_shared<Multiply>(
        std::make_shared<Add>(
            std::make_shared<Power>(x, std::make_shared<Constant>(2)),
            std::make_shared<Multiply>(std::make_shared<Constant>(3), x)),
        std::make_shared<Sin>(x));
    Expr deriv = expr->derivative();
    std::cout << "f(x) = " << expr->toString() << '\n';
    std::cout << "f'(x) = " << deriv->toString() << '\n';
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "f(2) = " << expr->evaluate(2) << '\n';
    std::cout << "f'(2) = " << deriv->evaluate(2) << '\n';
    return 0;
}
