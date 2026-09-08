#include "TestRunner.hpp"
#include "tools/ToolRegistry.hpp"
#include "tools/PrimitiveTools.hpp"
#include "tools/SemanticTools.hpp"

using namespace pixelforge;

TEST(ToolRegistryTest, ToolRegistrationAndSchema) {
    tools::ToolRegistry reg;
    tools::register_primitive_tools(reg);
    tools::register_semantic_tools(reg);

    ASSERT_TRUE(reg.has_tool("set_pixel"));
    ASSERT_TRUE(reg.has_tool("draw_line"));
    ASSERT_TRUE(reg.has_tool("create_outline"));
    ASSERT_FALSE(reg.has_tool("arbitrary_command_execution"));

    auto schema = reg.get_tools_schema();
    ASSERT_TRUE(schema.is_array());
    ASSERT_TRUE(schema.size() >= 30);
}

TEST(ToolRegistryTest, ParameterValidation) {
    tools::ToolRegistry reg;
    tools::register_primitive_tools(reg);

    sprite::Sprite spr(16, 16);
    sprite::History hist;
    tools::ExecutionContext ctx{.sprite = spr, .history = hist};

    // Missing required argument 'color'
    auto res_missing = reg.execute_tool("set_pixel", ctx, {{"x", 2}, {"y", 2}});
    ASSERT_FALSE(res_missing.success);

    // Wrong type: 'x' should be integer, given string
    auto res_wrong_type = reg.execute_tool("set_pixel", ctx, {{"x", "not_an_int"}, {"y", 2}, {"color", "#FF0000"}});
    ASSERT_FALSE(res_wrong_type.success);

    // Valid call
    auto res_valid = reg.execute_tool("set_pixel", ctx, {{"x", 2}, {"y", 2}, {"color", "#FF0000"}});
    ASSERT_TRUE(res_valid.success);
    ASSERT_EQ(spr.get_pixel(2, 2), core::Color(255, 0, 0, 255));
}

TEST(ToolRegistryTest, PermissionEnforcement) {
    tools::ToolRegistry reg;
    tools::register_primitive_tools(reg);

    // Revoke edit permission
    reg.revoke_permission("sprite:edit");

    sprite::Sprite spr(16, 16);
    sprite::History hist;
    tools::ExecutionContext ctx{.sprite = spr, .history = hist};

    auto res = reg.execute_tool("set_pixel", ctx, {{"x", 2}, {"y", 2}, {"color", "#FF0000"}});
    ASSERT_FALSE(res.success);
    ASSERT_TRUE(res.message.find("Permission denied") != std::string::npos);
}
