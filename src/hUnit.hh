<?hh
//
// Copyright 2018 Nathan Wehr. All rights reserved.
// See LICENSE.txt
// 

namespace hUnit;

require_once dirname(__FILE__) . "/SourceScanner.hh";
require_once dirname(__FILE__) . "/Assert.hh";

class hUnit {
    private Vector<\ReflectionClass> $suites;

    private int $numAssertions = 0;
    private int $numAssertionFailures = 0;

    public function __construct() {
        $classes = (new Vector(get_declared_classes()))->map((string $className) ==> {
            return new \ReflectionClass($className);
        });

        $this->suites = $this->filterClassesForTestSuites($classes);
    }

    private function filterClassesForTestSuites(Vector<\ReflectionClass> $classes) : Vector<\ReflectionClass> {
        return $classes->filter((\ReflectionClass $class) ==> {
            $attributes = new Map($class->getAttributes());

            if($attributes->containsKey("TestSuite") && !$attributes->containsKey("Skip")) {
                return true;
            } else {
                return false;
            }
        });
    }

    private function filterMethodsForTests(Vector<\ReflectionMethod> $methods) : Vector<\ReflectionMethod> {
        return $methods->filter((\ReflectionMethod $method) ==> {
            $attributes = new Map($method->getAttributes());

            if($attributes->containsKey("Test") && !$attributes->containsKey("Skip")) {
                return true;
            } else {
                return false;
            }
        });
    }

    private function printStats() : void {
        printf("Test Suites        : %d\n", $this->suites->count());
        printf("Assertions         : %d\n", $this->numAssertions);
        printf("Assertion Failures : %d\n", $this->numAssertionFailures);
    }

    private function printFailure(AssertResult $result) : void {
        printf("FAILED \n%s::%s at %s:%d\n\n", $result->class, $result->method, $result->file, $result->line);
    }

    public function handleSuccess(AssertResult $result) {
        ++$this->numAssertions;
    } 

    public function handleFailure(AssertResult $result) {
        ++$this->numAssertions;
        ++$this->numAssertionFailures;

        $this->printFailure($result);
    }

    public function run() : int {
        foreach($this->suites as $suite) {
            $instance = $suite->newInstance();

            $tests = $this->filterMethodsForTests(new Vector($suite->getMethods())); 
            
            foreach($tests as $test) {
                $assert = new Assert($suite->getName(), $test->getName());

                $assert->success->connect(inst_meth($this, "handleSuccess"));
                $assert->failure->connect(inst_meth($this, "handleFailure"));
                
                $test->invokeArgs($instance, [$assert]);
            }
        }

        $this->printStats();

        return $this->numAssertionFailures ? 1 : 0;
    }
}