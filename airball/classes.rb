module Airball
  class Scope
    attr_reader :vars

    def initialize(parent=nil)
      @parent = parent
      @vars = {}
    end

    def [](name)
      @vars[name] ||
      @parent && @parent[name]
    end

    def []=(name, value)
      @vars[name] = value # TODO only if it doesn't exist in this scope
    end
  end

  class Obj
    def self.airball_name
      name.split('::').last
    end

    def to_s
      "<#{self.class.airball_name}>"
    end
  end

  class Int < Obj
    iattr :val

    def eval(scope)
      self
    end

    def to_s
      val.to_s
    end
  end

  class Str < Obj
    iattr :val

    def eval(scope)
      self
    end

    def to_s
      val
    end
  end

  class List < Obj
    iattr :vals

    def eval(scope)
      @vals.map! { |v| v.eval(scope) }
      self
    end

    def to_s
      vals.map { |v| v.to_s }.join("\n")
    end
  end

  class True < Obj
    def eval(scope)
      self
    end

    def val
      true
    end

    def to_s
      "true"
    end
  end

  class False < Obj
    def eval(scope)
      self
    end

    def val
      false
    end

    def to_s
      "false"
    end
  end

  class Function < Obj
    attr_accessor :args, :scope

    def initialize(args, body=nil, &body_proc)
      @args      = args
      @body      = body
      @body_proc = body_proc
    end

    def call(args, scope)
      # args come from calling scope
      args = args.map { |a| a.eval(scope) }
      # function body scope is based on the closing (lexical) scope of the function
      scope = Scope.new(@scope)
      @args.each_with_index { |a, i| scope[a] = args[i] }
      if @body_proc
        @body_proc.call(scope, *args)
      else
        @body[0..-2].each do |expr|
          expr.eval(scope)
        end
        @body.last.eval(scope) # return last result from function
      end
    end

    def eval(scope)
      @scope = scope
      self
    end
  end

  class Call < Obj
    iattr :name, :args

    def eval(scope)
      function = scope[name]
      if Function === function
        if DEBUG and DEBUG.include?(:calls)
          puts "DEBUG: calling '#{name}' with args #{args.inspect}" 
          result = function.call(args, scope)
          puts "DEBUG: result => #{result}"
          result
        else
          function.call(args, scope)
        end
      else
        raise Errors::FunctionNotFound, "'#{name}' could not be found in the current scope."
      end
    end

    def to_s
      "<Call #{name}>"
    end
  end

  class Assign < Obj
    iattr :name, :val

    def eval(scope)
      scope[name] = val.eval(scope)
    end
  end

  class Var < Obj
    iattr :name

    def eval(scope)
      scope[name]
    end

    def to_s
      "<Var #{name}>"
    end
  end
end
