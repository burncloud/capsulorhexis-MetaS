#pragma once

#include <c10/util/ArrayRef.h>
#include <c10/util/Exception.h>
#include <c10/util/StringUtil.h>
#include <string>

namespace c10 {

// Represents a name of the form "foo.bar.baz"
struct QualifiedName {
  QualifiedName() {}

  // `name` can be a dotted string, like "foo.bar.baz", or just a bare name.
  /* implicit */ QualifiedName(const std::string& name) {
    TORCH_CHECK(!name.empty());
    // split the string into its atoms.
    size_t startSearchFrom = 0;
    size_t pos = name.find(delimiter_, startSearchFrom);

    while (pos != std::string::npos) {
      auto atom = name.substr(startSearchFrom, pos - startSearchFrom);
      AT_ASSERTM(
          atom.size() > 0, "Invalid name for qualified name: '", name, "'");
      atoms_.push_back(std::move(atom));
      startSearchFrom = pos + 1;
      pos = name.find(delimiter_, startSearchFrom);
    }

    auto finalAtom = name.substr(startSearchFrom, pos - startSearchFrom);
    AT_ASSERTM(
        finalAtom.size() > 0, "Invalid name for qualified name: '", name, "'");
    atoms_.push_back(std::move(finalAtom));

    cacheAccessors();
  }

  explicit QualifiedName(std::vector<std::string> atoms) {
    for (const auto& atom : atoms) {
      TORCH_CHECK(!atom.empty(), "Atom cannot be empty");
      TORCH_CHECK(
          atom.find(delimiter_) == std::string::npos,
          "Delimiter not allowed in atom");
    }
    atoms_ = atoms;
    cacheAccessors();
  }
  // Unnecessary copy. Ideally we'd use somoething like std::string_view.
  /* implicit */ QualifiedName(const char* name)
      : QualifiedName(std::string(name)) {}

  // `name` must be a bare name (no dots!)
  explicit QualifiedName(const QualifiedName& prefix, std::string name) {
    TORCH_INTERNAL_ASSERT(!name.empty());
    TORCH_INTERNAL_ASSERT(name.find(delimiter_) == std::string::npos);
    atoms_.insert(atoms_.begin(), prefix.atoms_.begin(), prefix.atoms_.end());
    atoms_.push_back(std::move(name));

    cacheAccessors();
  }

  // Is `this` a prefix of `other`?
  // For example, "foo.bar" is a prefix of "foo.bar.baz"
  bool isPrefixOf(const QualifiedName& other) const {
    const auto& thisAtoms = atoms_;
    const auto& otherAtoms = other.atoms_;

    if (thisAtoms.size() > otherAtoms.size()) {
      // Can't be a prefix if it's bigger
      return false;
    }
    for (size_t i = 0; i < thisAtoms.size(); i++) {
      if (thisAtoms[i] != otherAtoms[i]) {
        return false;
      }
    }
    return true;
  }

  // The fully qualified name, like "foo.bar.baz"
  const std::string& qualifiedName() const {
    return qualifiedName_;
  }

  // The leading qualifier, like "foo.bar"
  const std::string& prefix() const {
    return prefix_;
  }

  // The base name, like "baz"
  const std::string& name() const {
    return name_;
  }

  const std::vector<std::string>& atoms() const {
    return atoms_;
  }

  bool operator==(const QualifiedName& other) const {
    return this->qualifiedName_ == other.qualifiedName_;
  }

  bool operator!=(const QualifiedName& other) const {
    return !(*this == other);
  }

 private:
  char delimiter_ = '.';

  void cacheAccessors() {
    qualifiedName_ = Join(std::string(1, delimiter_), atoms_);
    if (atoms_.size() > 1) {
      ArrayRef<std::string> view(atoms_);
      const auto prefixView = view.slice(0, view.size() - 1);
      prefix_ = Join(".", prefixView);
    }

    if (atoms_.size() >= 1) {
      name_ = atoms_.back();
    }
  }

  // The actual list of names, like "{foo, bar, baz}"
  std::vector<std::string> atoms_;

  /*
   * Cached accessors, derived from `atoms_`.
   */
  std::string qualifiedName_;
  std::string prefix_;
  std::string name_;
};
} // namespace c10

namespace std {
template <>
struct hash<c10::QualifiedName> {
  size_t operator()(const c10::QualifiedName& n) const noexcept {
    return std::hash<std::string>()(n.qualifiedName());
  }
};
} // namespace std
