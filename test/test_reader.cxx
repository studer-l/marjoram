#include "marjoram/reader.hpp"
#include "gtest/gtest.h"
#include <cmath>
#include <string>
#include <unordered_map>

using ma::Reader;

TEST(Reader, map) {
  Reader<int, double> sqrter([](int a) { return std::sqrt(a); });

  ASSERT_FLOAT_EQ(sqrter.run(9), 3.0);

  Reader<int, std::string> sqrtprinter =
      sqrter.map([](double s) { return std::to_string(s); });
  ASSERT_EQ(sqrtprinter.run(16), std::string("4.000000"));
}

TEST(Reader, flatMap) {
  Reader<int, double> sqrter([](int a) { return std::sqrt(a); });

  Reader<int, std::string> sqrtprinter = sqrter.flatMap([](double s) {
    return Reader<int, std::string>([s](int s2) {
      return std::string("orig = ") + std::to_string(s2) +
             std::string(", sqrt = ") + std::to_string(s);
    });
  });
  ASSERT_EQ(sqrtprinter.run(16), std::string("orig = 16, sqrt = 4.000000"));
}

/**
 * More involved test: Dependency Injection. Following the posting
 * http://blog.originate.com/blog/2013/10/21/reader-monad-for-dependency-injection/
 */

struct User {
  User(const User&) = delete;
  std::string firstName;
  std::string lastName;
  std::string email;
  int supervisorId;
};

class UserRepository {
 public:
  virtual const User& get(int id) const = 0;
  virtual const User& find(const std::string& username) const = 0;
};

/* can't use abstract parameters in std::function */
using UserRepositoryRef = std::reference_wrapper<const UserRepository>;

class Users {
 protected:
  /* note that here we painstakenling avoid copying the user by annotating
   * return type in the lambda as well as in the Reader definition */
  static Reader<UserRepositoryRef, const User&> getUser(int id) {
    return Reader<UserRepositoryRef, const User&>(
        [id](const UserRepositoryRef& ur) -> const User& {
          return ur.get().get(id);
        });
  }

  static Reader<UserRepositoryRef, const User&> findUser(
      const std::string& username) {
    return Reader<UserRepositoryRef, const User&>(
        [&username](const UserRepositoryRef& ur) -> const User& {
          return ur.get().find(username);
        });
  }
};

class UserInfo : Users {
 public:
  /* again insist on const ref */
  static Reader<UserRepositoryRef, std::string> userEmail(int id) {
    return getUser(id).map([](const User& u) { return u.email; });
  }

  static Reader<UserRepositoryRef, std::unordered_map<std::string, std::string>>
  userInfo(const std::string username) {
    /* that's when for comprehension / do notation would come in handy...  */
    return findUser(username).flatMap([](const User& user) {
      return getUser(user.supervisorId).map([&user](const User& boss) {
        std::unordered_map<std::string, std::string> r;
        r.emplace("fullName", user.firstName + " " + user.lastName);
        r.emplace("email", user.email);
        r.emplace("boss", boss.firstName + " " + boss.lastName);
        return r;
      });
    });
  }
};

/* So much fixture, so little test... */
class LegalUserRepository : public UserRepository {
 public:
  LegalUserRepository() : unknown{"John", "Doe", "john.doe@acme.corp", 0} {}

  const User& get(int) const override { return unknown; }

  const User& find(const std::string&) const override { return unknown; }

 private:
  User unknown;
};

TEST(Reader, DependencyInjection) {
  LegalUserRepository lur;
  auto johnmail = UserInfo::userEmail(0).run(lur);
  ASSERT_EQ(johnmail, "john.doe@acme.corp");
  auto johndata = UserInfo::userInfo("anything goes").run(lur);
  ASSERT_EQ(johndata["fullName"], "John Doe");
  ASSERT_EQ(johndata["email"], "john.doe@acme.corp");
  ASSERT_EQ(johndata["boss"], "John Doe");
}
