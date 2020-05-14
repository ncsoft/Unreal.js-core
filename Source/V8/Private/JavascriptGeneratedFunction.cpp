#include "JavascriptGeneratedFunction.h"
#include "JavascriptContext_Private.h"
#include "Engine/BlueprintGeneratedClass.h"

PRAGMA_DISABLE_SHADOW_VARIABLE_WARNINGS

DEFINE_FUNCTION(UJavascriptGeneratedFunction::Thunk)
{
	auto Function = static_cast<UJavascriptGeneratedFunction*>(Stack.CurrentNativeFunction);
	const auto ProcessInternalFunc = [&](FFrame& Stack, RESULT_DECL)
	{
		if (Function->JavascriptContext.IsValid())
		{
			TSharedPtr<FJavascriptContext> Ctx = Function->JavascriptContext.Pin();

			v8::Isolate::Scope isolate_scope(Ctx->isolate());
			v8::HandleScope handle_scope(Ctx->isolate());

			const bool bCallRet = Ctx->CallProxyFunction(Function->GetOuter(), P_THIS, Function, Stack.Locals);
			if (!bCallRet)
			{
				return;
			}

			FProperty* ReturnProp = ((UFunction*)Stack.Node)->GetReturnProperty();
			if (ReturnProp != nullptr)
			{
				const bool bHasReturnParam = Function->ReturnValueOffset != MAX_uint16;
				uint8* ReturnValueAdress = bHasReturnParam ? (Stack.Locals + Function->ReturnValueOffset) : nullptr;
				if (ReturnValueAdress)
					FMemory::Memcpy(RESULT_PARAM, ReturnValueAdress, ReturnProp->ArrayDim * ReturnProp->ElementSize);
			}

			bool bHasAnyOutParams = false;
			if (Function && Function->HasAnyFunctionFlags(FUNC_HasOutParms))
			{
				// Iterate over input parameters
				for (TFieldIterator<FProperty> It(Function); It && (It->PropertyFlags & (CPF_Parm | CPF_ReturnParm)) == CPF_Parm; ++It)
				{
					// This is 'out ref'!
					if ((It->PropertyFlags & (CPF_ConstParm | CPF_OutParm)) == CPF_OutParm)
					{
						bHasAnyOutParams = true;
						break;
					}
				}
			}

			if (bHasAnyOutParams)
			{
				auto OutParm = Stack.OutParms;
				if (OutParm)
				{
					// Iterate over parameters again
					for (TFieldIterator<FProperty> It(Function); It; ++It)
					{
						FProperty* Param = *It;

						auto PropertyFlags = Param->GetPropertyFlags();
						if ((PropertyFlags & (CPF_ConstParm | CPF_OutParm)) == CPF_OutParm)
						{
							auto Property = OutParm->Property;
							if (Property != nullptr)
							{
								auto ValueAddress = Property->ContainerPtrToValuePtr<uint8>(Stack.Locals);
								FMemory::Memcpy(OutParm->PropAddr, ValueAddress, Property->ArrayDim * Property->ElementSize);
							}
						}

						if (PropertyFlags & CPF_OutParm)
							OutParm = OutParm->NextOutParm;
					}
				}
			}
		}
	};

	const bool bIsVMVirtual = Function->GetSuperFunction() && Cast<UBlueprintGeneratedClass>(Function->GetSuperFunction()->GetOuter()) != nullptr;
	if (bIsVMVirtual && *Stack.Code != EX_EndFunctionParms)
	{
		uint8* Frame = NULL;
		/*#if USE_UBER_GRAPH_PERSISTENT_FRAME
				Frame = GetClass()->GetPersistentUberGraphFrame(this, Function);
		#endif*/
		const bool bUsePersistentFrame = (NULL != Frame);
		if (!bUsePersistentFrame)
		{
			Frame = (uint8*)FMemory_Alloca(Function->PropertiesSize);
			FMemory::Memzero(Frame, Function->PropertiesSize);
		}

		FFrame NewStack(P_THIS, Function, Frame, &Stack, Function->ChildProperties);
		FOutParmRec** LastOut = &NewStack.OutParms;
		FProperty* Property;

		// Check to see if we need to handle a return value for this function.  We need to handle this first, because order of return parameters isn't always first.
		if (Function->HasAnyFunctionFlags(FUNC_HasOutParms))
		{
			// Iterate over the function parameters, searching for the ReturnValue
			for (TFieldIterator<FProperty> ParmIt(Function); ParmIt; ++ParmIt)
			{
				Property = *ParmIt;
				if (Property->HasAnyPropertyFlags(CPF_ReturnParm))
				{
					CA_SUPPRESS(6263)
					FOutParmRec* RetVal = (FOutParmRec*)FMemory_Alloca(sizeof(FOutParmRec));

					// Our context should be that we're in a variable assignment to the return value, so ensure that we have a valid property to return to
					check(RESULT_PARAM != NULL);
					RetVal->PropAddr = (uint8*)RESULT_PARAM;
					RetVal->Property = Property;
					NewStack.OutParms = RetVal;

					// A function can only have one return value, so we can stop searching
					break;
				}
			}
		}

		for (Property = (FProperty*)Function->ChildProperties; *Stack.Code != EX_EndFunctionParms; Property = (FProperty*)Property->Next)
		{
			checkfSlow(Property, TEXT("NULL Property in Function %s"), *Function->GetPathName());

			Stack.MostRecentPropertyAddress = NULL;

			// Skip the return parameter case, as we've already handled it above
			const bool bIsReturnParam = ((Property->PropertyFlags & CPF_ReturnParm) != 0);
			if (bIsReturnParam)
			{
				continue;
			}

			if (Property->PropertyFlags & CPF_OutParm)
			{
				// evaluate the expression for this parameter, which sets Stack.MostRecentPropertyAddress to the address of the property accessed
				Stack.Step(Stack.Object, NULL);

				CA_SUPPRESS(6263)
				FOutParmRec* Out = (FOutParmRec*)FMemory_Alloca(sizeof(FOutParmRec));
				// set the address and property in the out param info
				// warning: Stack.MostRecentPropertyAddress could be NULL for optional out parameters
				// if that's the case, we use the extra memory allocated for the out param in the function's locals
				// so there's always a valid address
				ensure(Stack.MostRecentPropertyAddress); // possible problem - output param values on local stack are neither initialized nor cleaned.
				Out->PropAddr = (Stack.MostRecentPropertyAddress != NULL) ? Stack.MostRecentPropertyAddress : Property->ContainerPtrToValuePtr<uint8>(NewStack.Locals);
				Out->Property = Property;

				// add the new out param info to the stack frame's linked list
				if (*LastOut)
				{
					(*LastOut)->NextOutParm = Out;
					LastOut = &(*LastOut)->NextOutParm;
				}
				else
				{
					*LastOut = Out;
				}
			}
			else
			{
				// copy the result of the expression for this parameter into the appropriate part of the local variable space
				uint8* Param = Property->ContainerPtrToValuePtr<uint8>(NewStack.Locals);
				checkSlow(Param);

				Property->InitializeValue_InContainer(NewStack.Locals);

				Stack.Step(Stack.Object, Param);
			}
		}
		Stack.Code++;
#if UE_BUILD_DEBUG
		// set the next pointer of the last item to NULL so we'll properly assert if something goes wrong
		if (*LastOut)
		{
			(*LastOut)->NextOutParm = NULL;
		}
#endif

		if (!bUsePersistentFrame)
		{
			// Initialize any local struct properties with defaults
			for (FProperty* LocalProp = Function->FirstPropertyToInit; LocalProp != NULL; LocalProp = (FProperty*)LocalProp->Next)
			{
				LocalProp->InitializeValue_InContainer(NewStack.Locals);
			}
		}

		const bool bIsValidFunction = (Function->FunctionFlags & FUNC_Native) || (Function->Script.Num() > 0);

		// Execute the code.
		if (bIsValidFunction)
		{
			ProcessInternalFunc(NewStack, RESULT_PARAM);
		}

		if (!bUsePersistentFrame)
		{
			// destruct properties on the stack, except for out params since we know we didn't use that memory
			for (FProperty* Destruct = Function->DestructorLink; Destruct; Destruct = Destruct->DestructorLinkNext)
			{
				if (!Destruct->HasAnyPropertyFlags(CPF_OutParm))
				{
					Destruct->DestroyValue_InContainer(NewStack.Locals);
				}
			}
		}
	}
	else
	{
		P_FINISH;
		ProcessInternalFunc(Stack, RESULT_PARAM);
	}
}

PRAGMA_ENABLE_SHADOW_VARIABLE_WARNINGS
