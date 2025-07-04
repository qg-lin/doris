// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

package org.apache.doris.analysis;

import org.apache.doris.catalog.Env;
import org.apache.doris.catalog.Resource.ResourceType;
import org.apache.doris.common.AnalysisException;
import org.apache.doris.common.UserException;
import org.apache.doris.mysql.privilege.AccessControllerManager;
import org.apache.doris.mysql.privilege.PrivPredicate;
import org.apache.doris.qe.ConnectContext;

import com.google.common.collect.Maps;
import mockit.Expectations;
import mockit.Injectable;
import mockit.Mocked;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

import java.util.Map;

public class CreateResourceStmtTest {
    private Analyzer analyzer;
    private String resourceName1;
    private String resourceName3;

    @Before()
    public void setUp() {
        analyzer = AccessTestUtil.fetchAdminAnalyzer(true);
        resourceName1 = "spark0";
        resourceName3 = "s3";
    }

    @Test
    public void testNormal(@Mocked Env env, @Injectable AccessControllerManager accessManager)
            throws UserException {
        new Expectations() {
            {
                env.getAccessManager();
                result = accessManager;
                accessManager.checkGlobalPriv((ConnectContext) any, PrivPredicate.ADMIN);
                result = true;
            }
        };

        Map<String, String> properties = Maps.newHashMap();
        properties.put("type", "s3");
        CreateResourceStmt stmt = new CreateResourceStmt(true, false, resourceName3, properties);
        stmt.analyze(analyzer);
        Assert.assertEquals(resourceName3, stmt.getResourceName());
        Assert.assertEquals(ResourceType.S3, stmt.getResourceType());
        Assert.assertEquals("CREATE EXTERNAL RESOURCE 's3' PROPERTIES(\"type\"  =  \"s3\")", stmt.toSql());

    }

    @Test(expected = AnalysisException.class)
    public void testUnsupportedResourceType(@Mocked Env env, @Injectable AccessControllerManager accessManager)
            throws UserException {
        new Expectations() {
            {
                env.getAccessManager();
                result = accessManager;
                accessManager.checkGlobalPriv((ConnectContext) any, PrivPredicate.ADMIN);
                result = true;
            }
        };

        Map<String, String> properties = Maps.newHashMap();
        properties.put("type", "hadoop");
        CreateResourceStmt stmt = new CreateResourceStmt(true, false, resourceName1, properties);
        stmt.analyze(analyzer);
    }
}
