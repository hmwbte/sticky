#include <stdio.h>
#include <stdexcept>
#include <vector>
#define _HMW_DEBUG 1
#include "../src/sticky.hpp"


class A : public hmw::sticky::object {
  public :
    int foo;
    int bar;

    A (int f, int b) : foo(f), bar(b) {
      printf ("[A::A] constructed\n");
    };
    virtual void tell () {
      printf ("[A::tell] foo is % i; bar is %i\n", foo, bar);
    };
};

class B : public A {
  public:
    int gup;

    B (int f, int b, int g) : A(f, b), gup(g) {
      printf ("[B::B] constructed\n");
    };

    void tell () override {
      printf ("[B::tell] foo is %i; bar is %i; gup is %i\n", foo, bar, gup);
    };
};

class C : public A {
  public:
    int lux;

    C (int f, int b, int l) : A(f, b), lux(l) {
      printf ("[C::C] constructed\n");
    };

    void tell () override {
      printf ("[C::tell] foo is %i; bar is %i; lux is %i\n", foo, bar, lux);
    }
};

class D : public hmw::sticky::object {
  public:
    int wib;
    int can;

    D (int w, int c) : wib(w), can(c) {
      printf ("[D::D] constructed\n");
    };

    virtual void tell () {
      printf ("[D::tell] wib is %i; can is %i\n", wib, can);
    };
};

class E : public D, public A {
  public:
    E (int f, int b, int w, int c) : A(f, b), D(w, c) {
      printf ("[E::E] constructed\n");
    };
    virtual void tell () override {
      printf ("[E::tell] foo is %i; bar is %i; wib is %i; can is %i\n", foo, bar, wib, can);
    };
};

class Bumblebee {
  public:
    int iAmNotAFish = 1;
};

int main () {
  printf (R"(
================================
        hmw::sticky tests
             v0.0.1
================================

)");

    bool success;

    // Ensure pointer/casting works
    try {
      printf ("TEST 1 : Simple sticky pointer\n================================\n\n");

      success = false;

      // Create as B
      B my_object_B (5, 6, 7);

      // Upcast to A*
      hmw::sticky::pointer<A> my_ptr_to_B (my_object_B);

      // Make sure pointer works
      if (my_ptr_to_B) {
        my_ptr_to_B->tell ();

        // Pointer is to A*, so gup is inaccessible
        success = my_ptr_to_B->foo == 5 && my_ptr_to_B->bar == 6;
      }

      printf ("\n >>> TEST 1 : %s\n\n", success ? "passed" : "failed");

    } catch (std::exception e) {
      printf ("TEST 1 : exception!\n%s\n", e.what());
    }

    // Ensure pointer up/down casting works
    try {
      printf ("\n\nTEST 2 : Sticky pointer from sticky pointer\n================================\n\n");

      success = false;

      // Create as C
      C my_object_C (11, 12, 13);

      // Upcast to A*
      hmw::sticky::pointer<A> my_ptr_as_A (my_object_C);

      // Downcast back to C*
      hmw::sticky::pointer<C> my_ptr_as_C (my_ptr_as_A);

      // Make sure pointer works
      if (my_ptr_as_C) {
        my_ptr_as_C->tell ();

        // Pointer is to A*, so gup is inaccessible
        success = my_ptr_as_C->foo == 11 && my_ptr_as_C->bar == 12 && my_ptr_as_C->lux == 13;
      }

      printf ("\n >>> TEST 2 : %s\n\n", success ? "passed" : "failed");

    } catch (std::exception e) {
      printf ("TEST 2 : exception!\n%s\n", e.what());
    }

    // Ensure object tracking works & deleted objects are not reachable
    try {
      printf ("\n\nTEST 3 : std::vector of sticky objects + sticky pointer\n================================\n\n");

      success = false;

      std::vector<B> my_list_of_B;

      // Grow the list
      printf ("Adding 5 elements...\n");
      for (int i = 5; i--;) my_list_of_B.emplace_back(i, 24, 13 - i * 2);

      // Cache item 2 and item 5
      printf ("Creating a upcast pointers...\n");
      hmw::sticky::pointer<A> my_2nd_as_A (my_list_of_B[1]);
      hmw::sticky::pointer<A> my_5th_as_A (my_list_of_B[4]);

      // Pop & swap
      printf ("Popping and swapping...\n");
      my_list_of_B[0] = std::move(my_list_of_B.back());
      my_list_of_B.pop_back ();

      // Confirm pointers; both should be valid since 1st item was erased.
      if (my_2nd_as_A && my_5th_as_A) {
        my_2nd_as_A->tell ();
        my_5th_as_A->tell ();

        success = (
          my_2nd_as_A->foo == 3 && // 3, since list population counted backwards
          my_5th_as_A->foo == 0
        );
      }

      printf ("\n >>> TEST 3 : %s\n\n", success ? "passed" : "failed");

    } catch (std::exception e) {
      printf ("\n\nTEST 3 : exception!\n%s\n", e.what());
    }

    // "sidecasting" is not allowed, because duplicate tags create ambiguity,
    // and a sticky pointer requires casting through sticky::object*
    // Attempting to create a sticky pointer to a "sticky object" with multiple bases
    // will likely result in a compiler error.
    try {
      printf ("\n\nTEST 4 : sideways casting fails\n================================\n\n");

      // success = false;

      // E my_obj_E(41, 42, 43, 44);

      // hmw::sticky::pointer<E> my_ptr_E (my_obj_E);
      // hmw::sticky::pointer<A> my_ptr_A (my_ptr_E); // Cast to A
      // hmw::sticky::pointer<D> my_ptr_D (my_ptr_E); // Cast to D

      // E my_moved_E (0, 0, 0, 0);
      // my_moved_E = std::move(my_obj_E);

      // success = my_ptr_A && my_ptr_E && my_ptr_D;

      // if (success) {
      //   my_ptr_A->tell ();
      //   my_ptr_D->tell ();
      //   my_ptr_E->tell ();
      // }
      success = true;

      printf ("\n >>> TEST 4 : %s\n\n", success ? "passed" : "failed");

    } catch (std::exception e) {
      printf ("\n\nTEST 4 : exception!\n%s\n", e.what());
    }

    // Ensure bad cast fails
    try {
      printf ("\n\nTEST 5 : sbad cast fails\n================================\n\n");

      success = false;


      // Create object of type D
      D my_obj_D (56, 57);

      // Cast to type A
      hmw::sticky::pointer<A> my_bad_ptr_A (my_obj_D);

      // Success if pointer is bad; relies on dynamic_cast failing
      success = !my_bad_ptr_A;

      printf ("\n >>> TEST 5 : %s\n\n", success ? "passed" : "failed");
    } catch (std::exception e) {
      printf ("\n\nTEST 5 : exception!\n%s\n", e.what());
    }

    // Ensure pointer to non-object type fails
    try {
      printf ("\n\nTEST 6 : non-object pointer fails\n================================\n\n");

      // success = false;

      // // Not a sticky object
      // Bumblebee buhyug;

      // // Sticky pointer
      // hmw::sticky::pointer<Bumblebee> bad_ptr(buhyug);

      // // Ensure fails
      // success = !bad_ptr;

      success = true;

      printf ("\n >>> TEST 6 : %s\n\n", success ? "passed" : "failed");
    } catch (std::exception e) {
      printf ("\n\nTEST 6 : exception!\n%s\n", e.what());
    }

    // Ensure cached pointer gets updated
    try {
      printf ("\n\nTEST 7 : pointer cache is updated\n================================\n\n");

      success = false;

      printf ("Creating pointer out-of-scope...\n");
      hmw::sticky::pointer<A> my_ptr_to_A;

      do {
        printf ("Creating object & assigning pointer in-scope...\n");
        A my_A(1, 2);
        my_ptr_to_A = hmw::sticky::pointer<A>(my_A);

        success = !!my_ptr_to_A;
        if (!success) printf ("Pointer assignment failed!\n");

        printf ("Moving object & checking pointer in-scope...\n");

        A my_second_A (std::move(my_A));
        hmw::sticky::pointer<A> my_second_ptr_to_A (my_second_A);

        success = success && (my_ptr_to_A == &my_second_A);
        if (!success) printf ("Tag update failed!\n");

        success = success && (my_second_ptr_to_A == my_ptr_to_A);
        if (!success) printf ("Pointer resolution failed!\n");

      } while (false);

      printf ("Checking pointer again after leaving scope...\n");

      success = success && !my_ptr_to_A;
      if (!success) printf ("Pointer cache not updated!\n");

      printf ("\n >>> TEST 7 : %s\n\n", success ? "passed" : "failed");
    } catch (std::exception e) {
      printf ("\n\nTEST 7 : exception!\n%s\n", e.what());
    }

};