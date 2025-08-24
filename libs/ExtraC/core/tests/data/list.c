#include "./data.h"


#define TestListFor(test_list, string) \
if(strncmp(List.GetPointer(test_list, 0), \
string,sizeof(string)) == 0)

#define TestListNotFor(test_list, string) \
if(strncmp(List.GetPointer(test_list, 0), \
string,sizeof(string)) != 0)

bool RUN_LIST_TESTS(){


NEW_TEST("List Data Structure"){
	List(c8) test_list = NULL;
	List(c8) sublist = NULL;

	NEW_SUBTEST("Initialization"){
		test_list = pushList(c8, 10);
		if(isinit(test_list))
			PASS_TEST
		else{ 
			FAIL_TEST
			goto skip;
		}
	}

	NEW_SUBTEST("Appending"){
		List.Append(test_list, "Hello, World!", sizeof("Hello, World!"));
	
		TestListFor(test_list, "Hello, World!")
			PASS_TEST
		else{ 
			FAIL_TEST
			goto skip;
		}
	}
	NEW_SUBTEST("Clearing"){
		List.Flush(test_list);
		if(List.Size(test_list) == 0)
			PASS_TEST
		else{
			FAIL_TEST
			goto skip;
		}

	}
	NEW_SUBTEST("Insertion"){
		List.Insert(test_list, sizeof("Hello, World!"), UINT64_MAX, "Hello, World!");
		
		TestListNotFor(test_list, "Hello, World!"){
			FAIL_TEST
			loginfo("FAIL RESULT: ", $((cstr)List.GetPointer(test_list,0)));
			goto skip;
		}
		
		List.Insert(test_list , sizeof("Other ") - 1, 7, "Other ");
		TestListNotFor(test_list, "Hello, Other World!"){
			FAIL_TEST
			loginfo("FAIL RESULT: ",$((cstr)List.GetPointer(test_list,0)));
			goto skip;
		}else 
			PASS_TEST
	}
	NEW_SUBTEST("Creating a Sublist"){
		sublist = List.SubList(test_list, 7, sizeof("Other ") - 1);
		TestListNotFor(sublist, "Other "){
			FAIL_TEST
			loginfo("FAIL RESULT: ",$((cstr)List.GetPointer(sublist,0)));
			goto skip;
		}
		List.Flush(sublist);
		//Tests both:
			//sublisting the entire test_list from index,
			//freeing and reinitialization of an already created test_list
		sublist = List.SubList(test_list, 7, UINT64_MAX);
		TestListNotFor(sublist, "Other World!"){
			FAIL_TEST
			loginfo("FAIL RESULT: ",$((cstr)List.GetPointer(sublist,0)));
			goto skip;
		}else 
			PASS_TEST
	}
	NEW_SUBTEST("Merging"){
		List.Flush(test_list);
		List.Insert(test_list, sizeof("Goodbye,  See you tommorrow!"),
	     		UINT64_MAX, "Goodbye,  See you tommorrow!");
		List.Pop(sublist, 1);// removing the null terminator
		List.Merge(test_list, sublist, 9);
		TestListNotFor(test_list, "Goodbye, Other World! See you tommorrow!"){
			FAIL_TEST
			loginfo("FAIL RESULT: ",$((cstr)List.GetPointer(test_list,0)));
			goto skip;
		}else 
			PASS_TEST
	}
	NEW_SUBTEST("Casting"){

	    quiet(
		
		ListCast(sublist, u32);
		
		if(List.Merge(test_list, sublist, 9) != DATAERR_SIZETOOLARGE){
			FAIL_TEST
			loginfo("FAIL RESULT: Incorrect error code returned");
			goto skip;
		}
		
		TestListNotFor(test_list, "Goodbye, Other World! See you tommorrow!"){
			FAIL_TEST
			loginfo("FAIL RESULT: ",$((char*)List.GetPointer(test_list,0)));
			goto skip;
		}else 
			PASS_TEST
	    )
	}
}
skip:

return TEST_RESULT; 
}
