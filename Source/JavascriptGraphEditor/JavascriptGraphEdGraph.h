#pragma once

#include "JavascriptGraphEdGraph.generated.h"

UCLASS()
class UJavascriptGraphEdGraph : public UEdGraph
{
	GENERATED_BODY()

public:
	UJavascriptGraphEdGraph();
	virtual ~UJavascriptGraphEdGraph();

	UFUNCTION()
	void RebuildGenericGraph();

#if WITH_EDITOR
	virtual void PostEditUndo() override;
#endif
};
