FirstLevelElement[3] array_of_elements
================================================================================
MSG: fastdds/FirstLevelElement
string useless_name
SecondLevelElement[] sub
ThirdLevelElement an_element_alone
================================================================================
MSG: fastdds/ThirdLevelElement
float64 x
float64 y
================================================================================
MSG: fastdds/SecondLevelElement
ThirdLevelElement an_element_alone
ThirdLevelElement[<=1] a_limited_other_value
