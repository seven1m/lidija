require File.expand_path('../../airball', __FILE__)
require 'test/unit'

class ListTest < Test::Unit::TestCase
  def execute(source)
    if @program
      @program.source = source
    else
      @program = Airball::Program.new(source)
    end
    @program.scope["stdout"] = out = StringIO.new
    @program.run
    out.string
  end

  def test_list
    output = execute("out [1 'string' 3 * 5 ]")
    assert_equal "1\nstring\n15\n", output
  end

  def test_first
    output = execute("l = [1 'string' 3 * 5]
                      out (first l)")
    assert_equal "1\n", output
  end

  def test_rest
    output = execute("l = [1 'string' 3 * 5]
                      out (rest l)")
    assert_equal "string\n15\n", output
  end

  def test_count
    output = execute("l = [1 'string' 3 * 5]
                      out (count l)")
    assert_equal "3\n", output
  end

  def test_concat
    output = execute("l = [1 'string'] + [3 * 5]
                      out l")
    assert_equal "1\nstring\n15\n", output
  end
end