require File.expand_path('../../airball', __FILE__)
require 'test/unit'

class ParserTest < Test::Unit::TestCase
  def setup
    @parser = Airball::Parser.new
  end

  def parse(body)
    @parser.parse(body)
  end

  def test_empty_line
    expected = "\n"
    actual = parse("\n")
    assert_equal expected, actual
  end

  def test_comment
    expected = [
      {:assign => {:name => "x",
                   :val => {:integer => "1"}}}
    ]
    actual = parse("# this is a comment\nx = 1\n# and another")
    assert_equal expected, actual
  end

  def test_int
    expected = [
      {:integer => "2"}
    ]
    actual = parse("2")
    assert_equal expected, actual
  end

  def test_op
    expected = [
      {:op => {:left => {:var => "x"}, :symbol => "*", :right => {:integer => "3"}}}
    ]
    actual = parse("x * 3")
    assert_equal expected, actual
  end

  def test_non_assign_symbols
    expected = [
      {:op => {:left => {:integer => "1"}, :symbol => "==", :right => {:integer => "1"}}}
    ]
    actual = parse("1 == 1")
    assert_equal expected, actual
  end

  def test_simple_call
    expected = [
      {:call => {:name => "foo", :args => [{:integer => "1"}, {:integer => "2"}]}},
      {:op   => {:left => {:var => "x"}, :symbol => "*", :right => {:integer => "3"}}}
    ]
    actual = parse("foo 1 2\nx * 3")
    assert_equal expected, actual
  end

  def test_multiline_call
    expected = [
      {:call => {:name => "foo", :args => [{:integer => "1"},
                                           {:integer => "2"}]}},
      {:call => {:name => "bar", :args => [{:integer => "3"},
                                           {:func => {:body => [{:integer => "4"}]}}]}}
    ]
    actual = parse("foo 1,
                        2
                    bar 3,
                        { 4 }")
    assert_equal expected, actual
  end

  def test_complex_call
    expected = [
      {:call => {:name => "foo",
                 :args => [{:op => {:left   => {:integer => "2"},
                                    :symbol => "*",
                                    :right  => {:integer => "3"}}}]}},
      {:call => {:name => "bar",
                 :args => [{:var => "baz"},
                           {:op => {:left   => {:integer => "4"},
                                    :symbol => "-",
                                    :right  => {:integer => "5"}}},
                           {:integer => "9"}]}}
    ]
    actual = parse("foo 2 * 3\nbar baz 4 - 5 9")
    assert_equal expected, actual
  end

  def test_no_arg_call
    expected = [
      {:call => {:name => "foo"}},
      {:call => {:name => "bar", :args => [{:call => {:name => "foo"}}]}}
    ]
    actual = parse("(foo)\nbar (foo)")
  end

  def test_nested_call
    expected = [
      {:op => {:left => {:op => {:left => {:var => "x"},
                                 :symbol => "*",
                                 :right => {:integer => "3"}}},
               :symbol => "-",
               :right => {:integer => "1"}}}
    ]
    actual = parse("(x * 3) - 1")
    assert_equal expected, actual

    expected = [
      {:op => {:left => {:var => "x"},
               :symbol => "*",
               :right => {:call => {:name => "foo",
                                    :args => [{:integer => "4"}]}}}},
    ]
    actual = parse("x * (foo 4)")
    assert_equal expected, actual

    expected = [
      {:op => {:left => {:var => "x"},
               :symbol => "*",
               :right => {:op => {:left => {:var => "x"},
                                  :symbol => "^",
                                  :right => {:op => {:left => {:var => "y"},
                                                     :symbol => "-",
                                                     :right => {:integer => "1"}}}}}}}
    ]
    actual = parse("x * (x ^ (y - 1))")
    assert_equal expected, actual
  end

  def test_assigns
    expected = [
      {:assign => {:name => "foo",
                   :val => {:integer => "3"}}},
      {:assign => {:name => "bar",
                   :val => {:var => "foo"}}},
      {:assign => {:name => "baz",
                   :val => {:call => {:name => "bap",
                                      :args => [{:integer => "3"}]}}}},
      {:assign => {:name => "^",
                   :val => {:var => "func"}}}
    ]
    actual = parse("foo = 3\nbar = foo\nbaz = bap 3\n^ = func")
    assert_equal expected, actual
  end

  def test_func
    expected = [
      {:func => {:body => [{:call => {:name => "foo",
                                      :args => [{:integer => "1"},
                                                {:integer => "2"}]}}]}}
    ]
    actual = parse("{ foo 1 2 }")
    assert_equal expected, actual
    actual = parse("{\n foo 1 2\n }")
    assert_equal expected, actual
  end

  def test_func_with_multiple_args
    expected = [
      {:func => {:args => [{:arg => "x"},
                           {:arg => "y"}],
                 :body => [{:call => {:name => "foo",
                                      :args => [{:var => "x"},
                                                {:var => "y"}]}}]}}
    ]
    actual = parse("[x y] { foo x y }")
    assert_equal expected, actual
    actual = parse("[ x y ]{\n foo x y\n }")
    assert_equal expected, actual
  end

  def test_func_with_single_arg
    expected = [
      {:func => {:args => [{:arg => "x"}],
                 :body => [{:call => {:name => "foo",
                                      :args => [{:var => "x"},
                                                {:var => "y"}]}}]}}
    ]
    actual = parse("[x] { foo x y }")
    assert_equal expected, actual
    actual = parse("[ x ]{\n foo x y\n }")
    assert_equal expected, actual
  end

  def test_assign_func
    expected = [
      {:assign => {
        :name => "foo",
        :val => {:func => {:args => [{:arg => "x"},
                                     {:arg => "y"}],
                           :body => [{:call => {:name => "bar",
                                                :args => [{:var => "x"},
                                                          {:var => "y"}]}}]}}}}
    ]
    actual = parse("foo = [x y] { bar x y }")
    assert_equal expected, actual
  end

  def test_func_return_func
    expected = [
      {:func => {:body => [{:func => {:args => [{:arg => "x"}],
                                      :body => [{:integer => "1"}]}}]}}
    ]
    actual = parse("{ [x] { 1 } }")
    assert_equal expected, actual
  end

  def test_leading_space_on_line
    expected = [
      {:var => "x"},
      {:var => "y"},
      {:var => "z"}
    ]
    actual = parse("x
                    y
                    z")
    assert_equal expected, actual
  end
end