#include "PermMapping.hpp"
#include "PermTable.hpp"

#include "ll/api/reflection/Deserialization.h"
#include "ll/api/reflection/Serialization.h"
#include <ll/api/io/FileUtils.h>

#include "mc/deps/core/string/HashedString.h"
#include "mc/world/item/VanillaItemNames.h"

#include "nlohmann/json.hpp"


namespace permc {


struct PermMapping::Impl {
    using FieldDescriptorPtr = std::shared_ptr<FieldDescriptor>;
    std::unordered_map<PermFieldName, FieldDescriptorPtr> const fieldOffset_;  // 1. 权限字段 => 权限表中的偏移量
    std::unordered_map<TypeName, PermFieldName>                 typeMapping_;  // 2. TypeName => 权限字段
    std::unordered_map<TypeName, FieldDescriptorPtr>            finalMapping_; // 3. TypeName => 权限表中的偏移量

    explicit Impl() {
        {
            PermTable  table{};
            auto&      unConstMap = const_cast<std::unordered_map<PermFieldName, FieldDescriptorPtr>&>(fieldOffset_);
            auto const baseAddr   = reinterpret_cast<char const*>(&table); // 基地址

            using EntryType = RolePerms::Entry; // 目标类型：视为“叶子”，直接记录 Offset，不继续递归
            auto visitor    = [&](auto&& self, auto& currentObj) -> void {
                ll::reflection::forEachMember(currentObj, [&](std::string_view name, auto& field) {
                    using field_type = std::remove_cvref_t<decltype(field)>;

                    // 情况 1: 是角色权限节点 (Entry) 或者是环境权限节点 (bool)
                    if constexpr (std::is_same_v<field_type, EntryType> || std::is_same_v<field_type, bool>) {
                        auto const   fieldAddr = reinterpret_cast<char const*>(&field);
                        size_t const offset    = fieldAddr - baseAddr;
                        unConstMap.emplace(
                            name,
                            std::make_shared<FieldDescriptor>(
                                offset,
                                sizeof(field_type),
                                std::same_as<field_type, bool> ? FieldType::Bool : FieldType::Entry
                            )
                        );
                    }
                    // 情况 2: 是其他结构体 (RolePerms, EnvironmentPerms)
                    else if constexpr (ll::reflection::is_reflectable_v<field_type>) {
                        self(self, field);
                    }
                });
            };
            visitor(visitor, table);
        }

#ifdef PERMC_DEBUG
        std::ostringstream oss;
        oss << "fieldOffset_:\n";
        for (auto const& [name, descriptor] : fieldOffset_) {
            oss << "  " << name.getString() << " => sizeof:" << descriptor->size << ", offset:" << descriptor->offset
                << ", type:" << magic_enum::enum_name(descriptor->type) << "\n";
        }
        std::cout << oss.str() << std::endl;
#endif
    }
};

PermMapping::PermMapping() : impl_(std::make_unique<Impl>()) {}
PermMapping::~PermMapping() = default;
PermMapping& PermMapping::get() {
    static PermMapping instance;
    return instance;
}
ll::Expected<> PermMapping::initTypeNameMapping(std::filesystem::path const& path) {
    initDefaultMapping();
    if (auto exp = loadMapping(path); !exp) {
        return exp;
    }
    if (auto exp = ensureMapping(); !exp) {
        return exp;
    }
    return compileFinalMapping();
}
optional_ref<PermMapping::FieldDescriptor> PermMapping::lookup(TypeName const& typeName) const {
    if (auto it = impl_->finalMapping_.find(typeName); it != impl_->finalMapping_.end()) {
        return it->second.get();
    }
    return std::nullopt;
}

#define REQUIRE_PERM_FIELD(FIELD)                                                                                      \
    static_assert(                                                                                                     \
        std::same_as<std::remove_cvref_t<decltype(std::declval<RolePerms>().FIELD)>, RolePerms::Entry>,                \
        "Please synchronize the '" #FIELD "' field in the mapping table"                                               \
    );

void PermMapping::initDefaultMapping() {
    impl_->typeMapping_.clear();

    REQUIRE_PERM_FIELD(allowPvP);
    REQUIRE_PERM_FIELD(allowHostileDamage);
    REQUIRE_PERM_FIELD(allowFriendlyDamage);
    REQUIRE_PERM_FIELD(allowSpecialEntityDamage);
    REQUIRE_PERM_FIELD(allowPlace);
    REQUIRE_PERM_FIELD(useBoneMeal);
    impl_->typeMapping_ = {
        /*实体关联权限*/
        {"minecraft:player",                     "allowPvP"                }, // 玩家
        /*敌对生物受伤*/
        {"minecraft:zombie",                     "allowHostileDamage"      }, // 僵尸
        {"minecraft:skeleton",                   "allowHostileDamage"      }, // 骷髅
        {"minecraft:creeper",                    "allowHostileDamage"      }, // 苦力怕
        {"minecraft:spider",                     "allowHostileDamage"      }, // 蜘蛛
        {"minecraft:enderman",                   "allowHostileDamage"      }, // 末影人
        {"minecraft:witch",                      "allowHostileDamage"      }, // 女巫
        {"minecraft:blaze",                      "allowHostileDamage"      }, // 烈焰人
        {"minecraft:ghast",                      "allowHostileDamage"      }, // 恶魂
        {"minecraft:magma_cube",                 "allowHostileDamage"      }, // 岩浆怪
        {"minecraft:silverfish",                 "allowHostileDamage"      }, // 银鱼
        {"minecraft:slime",                      "allowHostileDamage"      }, // 史莱姆
        {"minecraft:guardian",                   "allowHostileDamage"      }, // 守卫者
        {"minecraft:elder_guardian",             "allowHostileDamage"      }, // 长老守卫者
        {"minecraft:wither_skeleton",            "allowHostileDamage"      }, // 凋零骷髅
        {"minecraft:stray",                      "allowHostileDamage"      }, // 流浪者
        {"minecraft:husk",                       "allowHostileDamage"      }, // 干尸
        {"minecraft:zombie_villager",            "allowHostileDamage"      }, // 僵尸村民
        {"minecraft:drowned",                    "allowHostileDamage"      }, // 溺尸
        {"minecraft:phantom",                    "allowHostileDamage"      }, // 幻翼
        {"minecraft:pillager",                   "allowHostileDamage"      }, // 掠夺者
        {"minecraft:vindicator",                 "allowHostileDamage"      }, // 守卫者
        {"minecraft:ravager",                    "allowHostileDamage"      }, // 劫掠兽
        {"minecraft:evocation_illager",          "allowHostileDamage"      }, // 召唤师
        {"minecraft:vex",                        "allowHostileDamage"      }, // 幽灵
        {"minecraft:shulker",                    "allowHostileDamage"      }, // 潜影贝
        {"minecraft:endermite",                  "allowHostileDamage"      }, // 末影螨
        {"minecraft:cave_spider",                "allowHostileDamage"      }, // 洞穴蜘蛛
        {"minecraft:zoglin",                     "allowHostileDamage"      }, // 僵尸疣猪兽
        {"minecraft:piglin_brute",               "allowHostileDamage"      }, // 野猪人暴徒
        {"minecraft:hoglin",                     "allowHostileDamage"      }, // 疣猪兽
        {"minecraft:wither",                     "allowHostileDamage"      }, // 凋零
        {"minecraft:ender_dragon",               "allowHostileDamage"      }, // 末影龙
        /*友好生物*/
        {"minecraft:cow",                        "allowFriendlyDamage"     }, // 牛
        {"minecraft:pig",                        "allowFriendlyDamage"     }, // 猪
        {"minecraft:sheep",                      "allowFriendlyDamage"     }, // 羊
        {"minecraft:chicken",                    "allowFriendlyDamage"     }, // 鸡
        {"minecraft:rabbit",                     "allowFriendlyDamage"     }, // 兔子
        {"minecraft:mooshroom",                  "allowFriendlyDamage"     }, // 蘑菇牛
        {"minecraft:horse",                      "allowFriendlyDamage"     }, // 马
        {"minecraft:donkey",                     "allowFriendlyDamage"     }, // 驴
        {"minecraft:mule",                       "allowFriendlyDamage"     }, // 骡
        {"minecraft:ocelot",                     "allowFriendlyDamage"     }, // 猫
        {"minecraft:bat",                        "allowFriendlyDamage"     }, // 蝙蝠
        {"minecraft:sniffer",                    "allowFriendlyDamage"     }, // 探索者
        {"minecraft:camel",                      "allowFriendlyDamage"     }, // 骆驼
        {"minecraft:armadillo",                  "allowFriendlyDamage"     }, // 犰狳
        /*特殊实体*/
        {"minecraft:painting",                   "allowSpecialEntityDamage"}, // 画
        {"minecraft:hopper_minecart",            "allowSpecialEntityDamage"}, // 漏斗矿车
        {"minecraft:chest_boat",                 "allowSpecialEntityDamage"}, // 箱船
        {"minecraft:leash_knot",                 "allowSpecialEntityDamage"}, // 拴绳结
        {"minecraft:armor_stand",                "allowSpecialEntityDamage"}, // 盔甲架
        {"minecraft:minecart",                   "allowSpecialEntityDamage"}, // 矿车
        {"minecraft:command_block_minecart",     "allowSpecialEntityDamage"}, // 指令方块矿车
        {"minecraft:boat",                       "allowSpecialEntityDamage"}, // 船
        {"minecraft:ender_crystal",              "allowSpecialEntityDamage"}, // 末影水晶

        /*特殊物品/方块关联权限(此类物品/方块仅在与其它方块交互时触发)*/
        /*物品关联*/
        {"minecraft:skull",                      "allowPlace"              }, // 头颅
        {"minecraft:banner",                     "allowPlace"              }, // 旗帜
        {"minecraft:glow_ink_sac",               "allowPlace"              }, // 灯光染料
        {"minecraft:end_crystal",                "allowPlace"              }, // 末影水晶
        {"minecraft:ender_eye",                  "allowPlace"              }, // 末影之眼
        {"minecraft:armor_stand",                "allowPlace"              }, // 盔甲架
        {"minecraft:bone_meal",                  "useBoneMeal"             }, // 骨粉
        /*特殊方块关联*/
        {"minecraft:chest",                      "useContainer"            }, // 箱子
        {"minecraft:trapped_chest",              "useContainer"            }, // 漏斗箱
        {"minecraft:campfire",                   "useCampfire"             }, // 营火
        {"minecraft:soul_campfire",              "useCampfire"             }, // 灵魂营火
        {"minecraft:composter",                  "useComposter"            }, // 堆肥桶
        {"minecraft:noteblock",                  "useNoteBlock"            }, // 音符盒
        {"minecraft:jukebox",                    "useJukebox"              }, // 唱片机
        {"minecraft:bell",                       "useBell"                 }, // 钟
        {"minecraft:daylight_detector_inverted", "useDaylightDetector"     }, // 反转阳光探测器
        {"minecraft:daylight_detector",          "useDaylightDetector"     }, // 阳光探测器
        {"minecraft:lectern",                    "useLectern"              }, // 讲台
        {"minecraft:cauldron",                   "useCauldron"             }, // 炼药锅
        {"minecraft:respawn_anchor",             "useRespawnAnchor"        }, // 重生锚
        {"minecraft:flower_pot",                 "editFlowerPot"           }, // 花盆
        {"minecraft:sweet_berry_bush",           "allowDestroy"            }, // 甜莓丛
        /*功能类方块关联（合并至 useWorkstation / useContainer）*/
        {"minecraft:cartography_table",          "useWorkstation"          }, // 制图台
        {"minecraft:smithing_table",             "useWorkstation"          }, // 锻造台
        {"minecraft:brewing_stand",              "useWorkstation"          }, // 酿造台
        {"minecraft:anvil",                      "useWorkstation"          }, // 铁砧
        {"minecraft:grindstone",                 "useWorkstation"          }, // 砂轮
        {"minecraft:enchanting_table",           "useWorkstation"          }, // 附魔台
        {"minecraft:loom",                       "useWorkstation"          }, // 织布机
        {"minecraft:stonecutter_block",          "useWorkstation"          }, // 切石机
        {"minecraft:crafter",                    "useWorkstation"          }, // 合成器
        {"minecraft:chiseled_bookshelf",         "useContainer"            }, // 雕纹书架
        {"minecraft:barrel",                     "useContainer"            }, // 木桶/存储桶
        {"minecraft:hopper",                     "useContainer"            }, // 漏斗
        {"minecraft:dropper",                    "useContainer"            }, // 投掷器
        {"minecraft:dispenser",                  "useContainer"            }, // 发射器
        {"minecraft:vault",                      "useContainer"            }, // 宝库
        {"minecraft:beacon",                     "useBeacon"               }, // 信标
        {"minecraft:cake",                       "useCake"                 }, // 蛋糕
        {"minecraft:unpowered_comparator",       "useComparator"           }, // 红石比较器（未充能）
        {"minecraft:powered_comparator",         "useComparator"           }, // 红石比较器（充能）
        {"minecraft:unpowered_repeater",         "useRepeater"             }, // 红石中继器（未充能）
        {"minecraft:powered_repeater",           "useRepeater"             }, // 红石中继器（充能）
        {"minecraft:bee_nest",                   "useBeeNest"              }, // 蜂巢
        {"minecraft:beehive",                    "useBeeNest"              }, // 蜂箱
    };
}
ll::Expected<> PermMapping::loadMapping(std::filesystem::path const& path) {
    auto defaultJson = ll::reflection::serialize<nlohmann::json>(impl_->typeMapping_);
    if (!defaultJson) {
        return ll::makeStringError(defaultJson.error().message());
    }

    if (!std::filesystem::exists(path)) {
        auto dump = defaultJson.value().dump(2);
        if (!ll::file_utils::writeFile(path, dump)) {
            return ll::makeStringError("failed to write " + path.string());
        }
        return {};
    }

    auto rawJson = ll::file_utils::readFile(path);
    if (!rawJson) {
        return ll::makeStringError("failed to read " + path.string());
    }
    try {
        auto json = nlohmann::json::parse(*rawJson);
        defaultJson->merge_patch(json);
        if (auto expected = ll::reflection::deserialize(impl_->typeMapping_, defaultJson.value()); !expected) {
            return expected;
        }
        return {};
    } catch (nlohmann::json::exception const& exception) {
        return ll::makeStringError(exception.what());
    }
}
ll::Expected<> PermMapping::ensureMapping() {
    InvalidPermFieldNameError::InvalidPermFieldNames invalidNames;

    for (auto const& [typeName, fieldName] : impl_->typeMapping_) {
        if (impl_->fieldOffset_.find(fieldName) == impl_->fieldOffset_.end()) {
            invalidNames.push_back(fieldName);
        }
    }

    if (!invalidNames.empty()) {
        return ll::makeError<InvalidPermFieldNameError>(std::move(invalidNames));
    }

    return {};
}
ll::Expected<> PermMapping::compileFinalMapping() {
    for (auto const& [typeName, fieldName] : impl_->typeMapping_) {
        auto iter = impl_->fieldOffset_.find(fieldName);
        if (iter != impl_->fieldOffset_.end()) {
            impl_->finalMapping_[typeName] = iter->second;
        }
    }
    return {};
}


InvalidPermFieldNameError::InvalidPermFieldNameError(InvalidPermFieldNames invalidPermFieldNames)
: invalidPermFieldNames(std::move(invalidPermFieldNames)) {}
InvalidPermFieldNameError::~InvalidPermFieldNameError() = default;
std::string InvalidPermFieldNameError::message() const noexcept {
    std::ostringstream oss;
    oss << "invalid permission field name: ";
    for (auto const& name : invalidPermFieldNames) {
        oss << name.getString() << ", ";
    }
    return oss.str();
}

} // namespace permc